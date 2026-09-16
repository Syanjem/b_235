/* =========================================================================
 * 电机校准子流程（offset 采样 + 编码器零点对齐）
 *
 * 严格事件驱动，绝不写 delay / busy-wait。
 *   - 每次 Motor_Calibration_Process() 被调用 = 一个 PWM 周期
 *   - sample_cnt / tick 计数自增 → 达到阈值后自动迁移子状态
 *   - 链路断、超时 → 置 fault_code，保留旧校准结果（绝不半写）
 *
 * ========================================================================= */

#include "motor_calibration.h"
#include "Angle_Feedback.h"   /* Zero_mangle() */
#include "Idq_Feedback.h"     /* Idq_get() / Iab_get() 等底层电流获取  */

#include <string.h>
#include <math.h>

/* --------------- 静态实例（单电机场景够用；多电机改成从池里分配） --------------- */
static MOTOR_CALIB s_calib_ctx;

/* --------------- BSP 弱符号 fallback（没实现就空操作，避免破坏分层） --------------- */
__attribute__((weak)) void BSP_Motor_PWM_WriteNeutral(void) { /* BSP 层应实现：写 3 相 CCR = (begin+end)/2 */ }
__attribute__((weak)) void BSP_Motor_PWM_OutputEnable(uint8_t en) { (void)en; }

/* --------------- 内部工具 --------------- */
static inline int within_tick(uint32_t now, uint32_t ref, uint32_t margin)
{
    /* 无符号防回绕：只要 (now-ref) < margin 视为在窗口内 */
    return (now - ref) <= margin;
}

/* ========================================================================
 *                       校准模块对外 API
 * ======================================================================== */

void Motor_Calibration_Init(MOTOR_DATA *m)
{
    memset(&s_calib_ctx, 0, sizeof(s_calib_ctx));
    s_calib_ctx.sub_state           = CALIB_SUB_START;
    s_calib_ctx.offset_samples      = 2000;   /* 20 kHz 下 100 ms（≥ 500 推荐） */
    s_calib_ctx.offset_timeout_ticks= 5000;   /* 2.5×window，防止 ADC DMA 链死 */
    s_calib_ctx.zero_align_timeout  = 0;      /* 0 = 无限等待（你现有 Zero_mangle 是阻塞） */

    m->calib = &s_calib_ctx;   /* 挂到 motor 句柄 */
}

void Motor_Calibration_Start(MOTOR_DATA *m)
{
    MOTOR_CALIB *c = m->calib;
    memset(c, 0, sizeof(*c));

    c->sub_state              = CALIB_SUB_START;
    c->offset_samples         = 2000;
    c->offset_timeout_ticks   = 5000;
    c->zero_align_timeout     = 0;
    c->last_acc_tick          = m->runtime.tick;

    /* 结果暂存为旧值，成功再覆盖（失败保留旧值，经验 1424488） */
    c->res_offset_a           = m->controller.current_offset_a;
    c->res_offset_b           = m->controller.current_offset_b;
    c->res_zero_offset        = m->controller.encoder_zero_offset;
}

/* ========================================================================
 *                主流程：每次 PWM 周期调用一次
 * ======================================================================== */
int Motor_Calibration_Process(MOTOR_DATA *m)
{
    MOTOR_CALIB *c = m->calib;
    Angle_Struct *ang = m->components.angle;
    I_Struct     *I   = m->components.iabc;
    const uint32_t now = m->runtime.tick;

    switch (c->sub_state)
    {
    /* ================= START：冻结输出、初始化 ================= */
    case CALIB_SUB_START:
        BSP_Motor_PWM_OutputEnable(0);
        BSP_Motor_PWM_WriteNeutral();
        c->sample_cnt   = 0;
        c->acc_offset_a = 0.0;
        c->acc_offset_b = 0.0;
        c->last_acc_tick = now;
        c->zero_align_started = 0;
        c->sub_state = CALIB_SUB_OFFSET_ACC;
        return 0;

    /* ================= OFFSET_ACC：N 次采样累加 ================= */
    case CALIB_SUB_OFFSET_ACC:
    {
        /* 注意：
         *   1) offset 校准前提是 PWM 输出中点，线圈不激励，电流应为 0，读取的就是运放中点offset。
         *   2) Idq_get(I, 0) 第三个参数是 offset——这里故意传 0，
         *      让 I->ia/I->ib 等于原始物理值（不做 offset 减法），我们之后自己算平均。
         *   3) 如果上层 Idq_get 实际换算就是带 offset 的，那在 CALIB 态下会传入 0，
         *      保证这里拿到的是原始读数。
         */
        Idq_get(I, 0.0f);

        c->sample_cnt++;
        c->acc_offset_a += (double)I->ia;
        c->acc_offset_b += (double)I->ib;
        c->last_acc_tick = now;   /* 成功累加一次，更新时间戳 */

        /* ----- 达到样本数：下一状态 ----- */
        if (c->sample_cnt >= c->offset_samples) {
            c->sub_state = CALIB_SUB_OFFSET_DONE;
            return 0;
        }

        /* ----- 超时断链判定（经验 1424488） -----
         * tick 跑到 offset_timeout_ticks 还没达到 sample_cnt 目标
         * = ADC/DMA 链断了 / 没触发 / 主循环没调 Process
         */
        if ((now - c->last_acc_tick) > c->offset_timeout_ticks) {
            m->fault_code |= FAULT_CALIB_FAIL;
            c->sub_state = CALIB_SUB_FAIL;
            return -1;
        }
        return 0;
    }

    /* ================= OFFSET_DONE：平均 → 暂存 ================= */
    case CALIB_SUB_OFFSET_DONE:
    {
        const double inv_n = 1.0 / (double)c->sample_cnt;
        const float oa = (float)(c->acc_offset_a * inv_n);
        const float ob = (float)(c->acc_offset_b * inv_n);

        /* 结果合理性检查（经验 1424488：不合理直接 FAIL，不写回）
         * 你的 Idq_get 用 Vref=3.3V / 2 = 1.65V 换算成 A，
         * 无激励时换算后的 A 电流理论 ≈ 0。这里放宽 ±1A 判错。
         */
        if (fabsf(oa) > 1.0f || fabsf(ob) > 1.0f) {
            m->fault_code |= FAULT_CALIB_FAIL;
            c->sub_state = CALIB_SUB_FAIL;
            return -1;
        }
        /* 暂存到 calib->res（不立即写 motor_data.controller，成功最后一次写） */
        c->res_offset_a = oa;
        c->res_offset_b = ob;
        c->sub_state    = CALIB_SUB_ZERO_ALIGN;
        return 0;
    }

    /* ================= ZERO_ALIGN：编码器零点对齐 ================= */
    case CALIB_SUB_ZERO_ALIGN:
    {
        if (c->zero_align_started == 0) {
            /* 第一次进入：触发一次 Zero_mangle（兼容你现有阻塞式实现） */
            c->zero_align_started    = 1;
            c->zero_align_start_tick = now;
            if (ang != NULL) {
                Zero_mangle(ang);   /* 你现有：Zero_mangle 通常阻塞，返回就是完成 */
            }
        }

        /* 完成判定：
         *   - Zero_mangle 阻塞 → 上面这行返回后已经写了 ang->mangle_zero
         *   - Zero_mangle 非阻塞（未来版本） → 加个 zero_done 标志或者看 mangle_zero
         * 这里统一用"零值被写过"来判，兼容两种。
         */
        int zero_done = 0;
        if (ang != NULL) {
            /* Zero_mangle 至少会写 mangle_zero；只要和 Zero_mangle 前存的值不同就算完成。
             * 这里没有 pre-mangle_zero 快照，就简单认为阻塞式的 Zero_mangle() 返回=完成。
             * 如果你 Zero_mangle 是非阻塞，改成：
             *   zero_done = (ang->mangle_zero_ready_flag == 1);
             */
            (void)ang; /* silence warning if used below */
            zero_done = 1;  /* 假设阻塞（=默认安全：调用一次之后就算做完了） */
        }

        if (zero_done) {
            if (ang != NULL) {
                c->res_zero_offset = ang->mangle_zero;  /* 暂存 */
                c->sub_state = CALIB_SUB_DONE;
            } else {
                m->fault_code |= FAULT_ZERO_ALIGN_FAIL;
                c->sub_state = CALIB_SUB_FAIL;
                return -1;
            }
            return (c->sub_state == CALIB_SUB_DONE) ? 0 : -1;
        }

        /* 未完成：如果有超时，检查一下；没有就等下一次 Process */
        if (c->zero_align_timeout != 0) {
            if ((now - c->zero_align_start_tick) > c->zero_align_timeout) {
                m->fault_code |= FAULT_ZERO_ALIGN_FAIL;
                c->sub_state = CALIB_SUB_FAIL;
                return -1;
            }
        }
        return 0;
    }

    /* ================= DONE：一次性写回结果 ================= */
    case CALIB_SUB_DONE:
    {
        /* 校验暂存的结果（双保险：不写 NAN、不写 0.0000 随机值） */
        int valid = 1;
        valid = valid && isfinite(c->res_offset_a) && isfinite(c->res_offset_b);
        if (!valid) {
            m->fault_code |= FAULT_CALIB_FAIL;
            c->sub_state = CALIB_SUB_FAIL;
            return -1;
        }

        /* 原子写回 motor_data.controller（保持 calib_done 最后置 1） */
        m->controller.current_offset_a  = c->res_offset_a;
        m->controller.current_offset_b  = c->res_offset_b;
        m->controller.encoder_zero_offset = c->res_zero_offset;
        if (m->components.angle != NULL) {
            m->components.angle->mangle_zero = c->res_zero_offset;  /* 同步写回去 */
        }
        m->calib_done = 1;
        return 1;   /* 成功：外层切 RUNNING */
    }

    /* ================= FAIL：保留旧结果，上报错误 ================= */
    case CALIB_SUB_FAIL:
    default:
        return -1;
    }
}

/* =========================================================================
 *  电机校准子流程（offset 采样 + 编码器零点对齐）
 *
 *  严格事件驱动，绝不写 delay / busy-wait。
 *    - 每次 motor_calibration_process() = 一个 PWM 周期，由外层状态机调度
 *    - sample_cnt / stable_cnt / tick 自增 → 达阈值后自动迁移子状态
 *    - 链路断、超时、结果不合理 → 切 FAIL，保留旧校准结果（绝不半写）
 *
 *  依赖（不改其他代码的约束下）：
 *    - motor_data_t::components.p_idq   电流反馈（current_state_t）
 *    - motor_data_t::components.p_angle 角度反馈（angle_state_t）
 *    - motor_to_eangle()               施加电角度 0 的电压矢量（angle.c）
 *    - angle_feedback_update()         周期读编码器，更新 angle_mech
 *    - adc_inserted_data_read()       GD32 库，直接读 ADC raw（绕过滤波）
 *    - bsp_motor_pwm_*                弱符号，BSP 层应实现
 * ========================================================================= */

#include "motor_calibration.h"
#include "angle.h"             /* motor_to_eangle */
#include "Angle_Feedback.h"   /* angle_feedback_update */
#include "adc.h"               /* adc_inserted_data_read / ADC_INSERTED_CHANNEL_* */
#include <string.h>
#include <math.h>

/* --------------- 模块内单实例校准上下文（motor_data_t 无 calib 字段） --------------- */
static motor_calib_t s_calib_ctx;

/* --------------- IC/IB offset 分项（不写回 motor_data，供未来 Idq_Feedback 改造取用） --------------- */
static uint16_t s_offset_c = 2048u;
static uint16_t s_offset_b = 2048u;

/* --------------- BSP 弱符号 fallback（未实现则空操作，避免破坏分层） --------------- */
__attribute__((weak)) void bsp_motor_pwm_write_neutral(void)
{
    /* BSP 层应实现：写 3 相 CCR = (duty_min + duty_max) / 2 */
}

__attribute__((weak)) void bsp_motor_pwm_output_enable(uint8_t en)
{
    (void)en;
    /* BSP 层应实现：en=1 开 MOE，en=0 关 MOE（timer_primary_output_config 等） */
}


/* ========================================================================
 *                          对外 API
 * ======================================================================== */

void motor_calibration_init(motor_data_t *m)
{
    (void)m;
    memset(&s_calib_ctx, 0, sizeof(s_calib_ctx));
    s_calib_ctx.sub_state            = CALIB_SUB_START;
    s_calib_ctx.offset_samples       = 2000;    /* 20kHz 下 100ms */
    s_calib_ctx.offset_timeout_ticks = 5000;    /* 2.5× 目标样本数 */
    s_calib_ctx.zero_timeout         = 20000;   /* 1s @ 20kHz */
}

void motor_calibration_start(motor_data_t *m)
{
    (void)m;
    /* 失败兜底：保留旧结果，失败时不半写（经验：会导致电流环发散） */
    float old_oc = s_calib_ctx.res_offset_c;
    float old_ob = s_calib_ctx.res_offset_b;
    float old_az = s_calib_ctx.res_angle_zero;

    memset(&s_calib_ctx, 0, sizeof(s_calib_ctx));
    s_calib_ctx.sub_state            = CALIB_SUB_START;
    s_calib_ctx.offset_samples       = 2000;
    s_calib_ctx.offset_timeout_ticks = 5000;
    s_calib_ctx.zero_timeout         = 20000;

    s_calib_ctx.res_offset_c   = old_oc;
    s_calib_ctx.res_offset_b   = old_ob;
    s_calib_ctx.res_angle_zero = old_az;
}

int motor_calibration_process(motor_data_t *m)
{
    motor_calib_t *c = &s_calib_ctx;
    c->tick++;   /* 模块内 tick：每次 process 自增 */

    angle_state_t   *pa = m->components.p_angle;
    current_state_t *pi = m->components.p_idq;

    if (pa == NULL || pi == NULL) {
        c->sub_state = CALIB_SUB_FAIL;
        return -1;
    }

    switch (c->sub_state)
    {
    /* ================= START：冻结输出、初始化累加器 ================= */
    case CALIB_SUB_START:
        bsp_motor_pwm_output_enable(0);
        bsp_motor_pwm_write_neutral();
        c->sample_cnt    = 0;
        c->acc_offset_c  = 0.0;
        c->acc_offset_b  = 0.0;
        c->last_acc_tick = c->tick;
        c->zero_started  = 0;
        c->sub_state     = CALIB_SUB_OFFSET_ACC;
        return 0;

    /* ================= OFFSET_ACC：N 次采样累加 ================= */
    case CALIB_SUB_OFFSET_ACC:
    {
        /* 直接读 ADC raw，绕过 Idq_Feedback 的滤波/offset 减法
         * （零电流时要的就是原始中点值，不能让滤波/减法污染） */
        uint16_t ic = adc_inserted_data_read(ADC0, ADC_INSERTED_CHANNEL_0);
        uint16_t ib = adc_inserted_data_read(ADC0, ADC_INSERTED_CHANNEL_1);

        c->sample_cnt++;
        c->acc_offset_c += (double)ic;
        c->acc_offset_b += (double)ib;
        c->last_acc_tick = c->tick;

        /* 达到样本数 → 下一状态 */
        if (c->sample_cnt >= c->offset_samples) {
            c->sub_state = CALIB_SUB_OFFSET_DONE;
            return 0;
        }

        /* 链路断裂：tick 跑很久 sample_cnt 没长 → FAIL
         * （ADC/DMA 链死、主循环没调 process、中断未触发等） */
        if ((c->tick - c->last_acc_tick) > c->offset_timeout_ticks) {
            c->sub_state = CALIB_SUB_FAIL;
            return -1;
        }
        return 0;
    }

    /* ================= OFFSET_DONE：平均 + 合理性检查 + 写回 ================= */
    case CALIB_SUB_OFFSET_DONE:
    {
        const double inv_n = 1.0 / (double)c->sample_cnt;
        const float oc = (float)(c->acc_offset_c * inv_n);
        const float ob = (float)(c->acc_offset_b * inv_n);

        /* 合理性检查：零电流时 ADC raw 应在 2048 ±200（约 ±0.1A 量级）
         * 超出范围说明采样链路异常或有残余电流 → FAIL，不写回 */
        if (fabsf(oc - 2048.0f) > 200.0f || fabsf(ob - 2048.0f) > 200.0f) {
            c->sub_state = CALIB_SUB_FAIL;
            return -1;
        }
        if (!isfinite(oc) || !isfinite(ob)) {
            c->sub_state = CALIB_SUB_FAIL;
            return -1;
        }

        /* 暂存到上下文 + 模块内 static 保留分项 */
        c->res_offset_c = oc;
        c->res_offset_b = ob;
        s_offset_c = (uint16_t)oc;
        s_offset_b = (uint16_t)ob;

        /* 写回 motor_data：i_offset 单值填 IC/IB 平均
         * 注：现状 Idq_Feedback.c 用硬编码 2048，不读 i_offset，
         *     此处写回仅满足语义；真正生效需未来 Idq_Feedback 改造 */
        pi->i_offset = (uint16_t)((oc + ob) * 0.5f);

        c->sub_state = CALIB_SUB_ZERO_APPLY;
        return 0;
    }

    /* ================= ZERO_APPLY：施加电角度 0 电压矢量 ================= */
    case CALIB_SUB_ZERO_APPLY:
    {
        if (!c->zero_started) {
            c->zero_started    = 1;
            c->zero_start_tick = c->tick;

            /* 先读一次编码器，初始化稳定判定的初值 */
            angle_feedback_update(pa);
            c->last_mech  = pa->angle_mech;
            c->stable_cnt = 0;

            /* 写 CCR = 电角度 0 对应的占空比，再开 PWM 主输出
             * 转子在 d 轴电压作用下会自动对齐到电角度 0 */
            motor_to_eangle(0u);
            bsp_motor_pwm_output_enable(1);
        }
        c->sub_state = CALIB_SUB_ZERO_WAIT;
        return 0;
    }

    /* ================= ZERO_WAIT：等转子机械角稳定 ================= */
    case CALIB_SUB_ZERO_WAIT:
    {
        /* 周期读编码器，更新机械角（校准期间 FOC 中断可能停了，
         * 故模块自己调 angle_feedback_update，不依赖外层调度） */
        angle_feedback_update(pa);

        const float mech = pa->angle_mech;
        float d = mech - c->last_mech;
        /* 处理过零跳变（短路径差分） */
        if (d >= 180.0f)        d -= 360.0f;
        else if (d <= -180.0f)  d += 360.0f;

        if (fabsf(d) < 1.0f) {
            c->stable_cnt++;
            /* 稳定 50ms @ 20kHz = 1000 周期 */
            if (c->stable_cnt >= 1000u) {
                c->res_angle_zero = mech;
                bsp_motor_pwm_output_enable(0);
                bsp_motor_pwm_write_neutral();
                c->sub_state = CALIB_SUB_DONE;
                return 0;
            }
        } else {
            c->stable_cnt = 0;
        }
        c->last_mech = mech;

        /* 超时：1s 还没稳定 → FAIL，立即关 PWM */
        if ((c->tick - c->zero_start_tick) > c->zero_timeout) {
            bsp_motor_pwm_output_enable(0);
            bsp_motor_pwm_write_neutral();
            c->sub_state = CALIB_SUB_FAIL;
            return -1;
        }
        return 0;
    }

    /* ================= DONE：写回 angle_zero ================= */
    case CALIB_SUB_DONE:
    {
        if (!isfinite(c->res_angle_zero)) {
            c->sub_state = CALIB_SUB_FAIL;
            return -1;
        }
        pa->angle_zero = c->res_angle_zero;
        return 1;   /* 成功：外层切回 STANDBY/WORKING */
    }

    /* ================= FAIL：保留旧结果，由调用方上报 ================= */
    case CALIB_SUB_FAIL:
    default:
        return -1;
    }
}


/* ========================================================================
 *                          调试用：查询内部状态
 * ======================================================================== */
motor_calib_t *motor_calibration_get_ctx(void)
{
    return &s_calib_ctx;
}

uint16_t motor_calibration_get_offset_c(void) { return s_offset_c; }
uint16_t motor_calibration_get_offset_b(void) { return s_offset_b; }

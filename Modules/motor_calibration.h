#ifndef _MOTOR_CALIBRATION_H_
#define _MOTOR_CALIBRATION_H_

#include "Motor_ctrl.h"

/* =========================================================================
 *  电机校准子流程：电流 offset 采样 + 编码器零点对齐
 *
 *  设计约束（不改其他代码）：
 *    - motor_data_t 无 calib 字段 → 校准上下文做成模块内 static 单实例
 *    - current_state_t.i_offset 是 uint16_t 单值，存不下 IC/IB 两个分项
 *      → 模块内 static 保留分项；i_offset 字段填平均值（兼容现状
 *        Idq_Feedback.c 用硬编码 2048 的写法，等未来改造再用分项）
 *    - motor_data_t 无 fault 字段 → 失败仅靠返回码上报，由外层处理
 *
 *  调用约定（外层主循环状态机）：
 *    1) 切到 STATE_MODE_CALIBRATING 时调一次 motor_calibration_start()
 *    2) 之后每个 PWM 周期调一次 motor_calibration_process()
 *       （校准逻辑是事件驱动，绝不写 delay / busy-wait）
 *    3) 返回值：1=成功（外层切回 STANDBY/WORKING）、0=进行中、-1=失败
 *
 *  经验教训（必须遵守）：
 *    - 失败时保留旧校准结果，绝不半写 NAN/0/随机值（会导致电流环发散）
 *    - ADC 链路断裂必须显式判定（超时未达采样目标 → FAIL）
 *    - 校准期间 PWM 主输出必须先关闭，避免误启动电机
 * ========================================================================= */

/* 校准子状态 */
typedef enum {
    CALIB_SUB_START = 0,        /* 入口：冻结 PWM 输出 */
    CALIB_SUB_OFFSET_ACC,      /* offset 累加：N 次采样 IC/IB 的 ADC raw */
    CALIB_SUB_OFFSET_DONE,     /* offset 平均 + 合理性检查 + 写回 i_offset */
    CALIB_SUB_ZERO_APPLY,      /* 施加电角度 0 的电压矢量，让转子对齐 */
    CALIB_SUB_ZERO_WAIT,       /* 等待转子机械角稳定 */
    CALIB_SUB_DONE,            /* 写回 angle_zero */
    CALIB_SUB_FAIL,            /* 失败：保留旧校准结果，由外层上报 */
} calib_sub_state_t;

/* 校准上下文（模块内单实例 static，因 motor_data_t 无 calib 字段） */
typedef struct {
    calib_sub_state_t sub_state;

    /* offset 累加配置 */
    uint32_t offset_samples;       /* 目标采样数（20kHz 下 2000=100ms） */
    uint32_t offset_timeout_ticks; /* 链路断超时（≥2.5×offset_samples） */
    uint32_t sample_cnt;          /* 已采样次数 */
    uint32_t last_acc_tick;       /* 上次成功累加的 tick（链路断检测） */
    double   acc_offset_c;        /* C 相 ADC raw 累加（double 防饱和） */
    double   acc_offset_b;        /* B 相 ADC raw 累加 */

    /* 零点对齐 */
    uint8_t  zero_started;        /* 是否已施加电压 */
    uint32_t zero_start_tick;    /* 对齐开始 tick */
    uint32_t zero_timeout;       /* 对齐超时（tick） */
    float    last_mech;           /* 上一次机械角（度），用于稳定判定 */
    uint32_t stable_cnt;         /* 连续稳定计数 */

    /* 结果暂存（成功才写回 motor_data，失败保留旧值） */
    float    res_offset_c;       /* ADC raw 平均 */
    float    res_offset_b;
    float    res_angle_zero;     /* 编码器零点（度） */

    /* 模块内 tick（每次 process 自增，模拟运行时节拍） */
    uint32_t tick;
} motor_calib_t;


/* ---------- API ---------- */

/* 初始化校准模块（上电调用一次，配置默认参数，不动 motor_data） */
void motor_calibration_init(motor_data_t *m);

/* 启动一次校准（保留旧结果作为失败兜底，清零运行时计数） */
void motor_calibration_start(motor_data_t *m);

/* 步进一次校准流程（每个 PWM 周期由外层状态机调用）
 * 返回：-1 失败、0 进行中、1 成功完成
 */
int motor_calibration_process(motor_data_t *m);


/* ---------- BSP 弱符号回调 ----------
 * BSP 层应实现；未实现时 fallback 为空操作（避免破坏分层）
 * 注意：若 BSP 未实现 bsp_motor_pwm_output_enable，PWM 不会真的开关，
 *       零点对齐阶段将无法施加电压，校准会超时失败。
 */
void bsp_motor_pwm_write_neutral(void);       /* 写 3 相 CCR = (begin+end)/2 */
void bsp_motor_pwm_output_enable(uint8_t en);  /* 开/关 PWM 主输出（MOE） */


/* ---------- 调试用：查询内部状态 ---------- */
motor_calib_t *motor_calibration_get_ctx(void);

/* 查询 IC/IB offset 分项（uint16_t ADC raw，供未来 Idq_Feedback 改造取用） */
uint16_t motor_calibration_get_offset_c(void);
uint16_t motor_calibration_get_offset_b(void);

#endif /* _MOTOR_CALIBRATION_H_ */

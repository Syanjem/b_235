#ifndef _MOTOR_CALIBRATION_H_
#define _MOTOR_CALIBRATION_H_

/* =========================================================================
 * 电机校准子流程（offset 采样 + 编码器零点对齐）
 *
 * 严格遵守：
 *   1) 绝不写 delay / busy-wait。所有等待 = 状态机 + tick/sample_cnt 计数。
 *   2) 每次 Process() 调用 = 一个 PWM 周期（由外层 MotorStateTask 保证频率）。
 *   3) 校准失败保留旧 offset，绝不写 0/NAN/随机值（经验教训）。
 *   4) ADC/DMA 链路断裂显式判定：超时后 sample_cnt 没到目标 → 退出 FAULT。
 *
 * ========================================================================= */

#include "FOCMotor.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---------- 校准子状态 ---------- */
typedef enum
{
    CALIB_SUB_START = 0,   /* 入口：清零累加器、冻结 PWM（写中点） */
    CALIB_SUB_OFFSET_ACC,  /* 电流 offset 累加：持续 N 个 PWM 周期 */
    CALIB_SUB_OFFSET_DONE, /* offset 累加完成 → 计算平均值 → 写 controller */
    CALIB_SUB_ZERO_ALIGN,  /* 编码器零点对齐：调 Zero_mangle() 一次 */
    CALIB_SUB_DONE,        /* 全部完成 → 返回 1，外层切 RUNNING */
    CALIB_SUB_FAIL,        /* 任一阶段失败 → 返回 -1，外层切 FAULT */
} calib_sub_state_t;

/* ---------- 校准运行时数据（每个 MOTOR_DATA 挂一份） ---------- */
typedef struct MOTOR_CALIB_
{
    calib_sub_state_t sub_state;    /* 当前校准子状态 */

    /* --- 配置（Motor_Calibration_Init 填） --- */
    uint32_t offset_samples;        /* offset 累加样本数（建议 500~2000 次） */
    uint32_t offset_timeout_ticks;  /* 超时判据（≥ 2×offset_samples，防止链路死） */
    uint32_t zero_align_timeout;    /* 零对齐超时 tick（阻塞式 Zero_mangle=0 即可） */

    /* --- offset 累加 --- */
    uint32_t sample_cnt;            /* 已有效采样次数（完成 = offset_samples） */
    uint32_t last_acc_tick;         /* 上次累加时的 tick（用于检测链路断裂） */
    double   acc_offset_a;          /* A 相 offset 累加（double 防饱和） */
    double   acc_offset_b;          /* B 相 offset 累加 */

    /* --- 零点对齐 --- */
    uint8_t  zero_align_started;    /* 是否已经触发过 Zero_mangle() */
    uint32_t zero_align_start_tick; /* 触发时刻 tick */

    /* --- 结果（成功） --- */
    float    res_offset_a;          /* 暂存，最后再写 motor_data.controller（防止半写） */
    float    res_offset_b;
    uint16_t res_zero_offset;

} MOTOR_CALIB;

/* ---------- API ---------- */

/**
 * @brief 校准模块初始化：把 calib 挂到 motor_data.calib，填默认参数
 * @note  请在 Motor_Init 中调用
 */
void Motor_Calibration_Init(MOTOR_DATA *m);

/**
 * @brief 启动一次校准（把 motor.state 从 IDLE 切到 CALIBRATING 时会调一次）
 * @note  不启动硬件，只初始化内部状态；硬件冻结 PWM 由外层 MotorStateTask 负责
 */
void Motor_Calibration_Start(MOTOR_DATA *m);

/**
 * @brief 步进一次校准流程（在 MotorStateTask 的 CALIBRATING 分支每次调用）
 * @return -1 失败（fault_code 已被写入）、0 进行中、1 成功完成
 */
int Motor_Calibration_Process(MOTOR_DATA *m);

/**
 * @brief 内部：写硬件 PWM 到 50% 中点（IDLE/FAULT/CALIB_START 统一调用）
 * @note  用弱符号声明，BSP 层应实现它；如果 BSP 暂未实现，会 fallback 为空函数
 *        （防止 Modules 直接写寄存器破坏分层）
 */
void BSP_Motor_PWM_WriteNeutral(void);

/**
 * @brief 内部：允许 / 禁止 PWM 输出更新（RUNNING=1，其他=0）
 */
void BSP_Motor_PWM_OutputEnable(uint8_t en);

#ifdef __cplusplus
}
#endif

#endif /* _MOTOR_CALIBRATION_H_ */

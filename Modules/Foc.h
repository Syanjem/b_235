#ifndef _FOC_H_
#define _FOC_H_

#include "gpio.h"
#include "ATK.h"

#include "Idq_Feedback.h"
#include "Angle_Feedback.h"

#include "pid_3loops.h"

#include "Motor_ctrl.h"

extern uint16_t ma_test;

extern volatile uint16_t ATK_data;
extern volatile uint16_t atk_num;

extern volatile uint8_t foc_mode;
extern volatile uint16_t num_send;
void ADC0_1_IRQHandler(void);


void foc_task(void);


void foc_begin_mode_choose_task(void);
void foc_switch_mode_task(void);











void foc_pi_task(FOC_RUNNING_CONTROL_MODE fcm);





















void foc_debug(uint8_t f_m, float target, uint32_t d1, uint32_t d2);


void foc_feedback_update(Pi_Data_Struct* pd, Angle_Struct* pa, Idq_Struct* pi);

extern volatile uint32_t num;
extern uint16_t dnum2;
extern uint16_t dnum1;
void foc_1loop_update(Pi_Para_Struct* pi3d, Pi_Para_Struct* pi3q, Pi_Data_Struct* p_data);
void foc_2loop_update(Pi_Para_Struct* pi2, Pi_Para_Struct* pi3q, Pi_Para_Struct* pi3d, Pi_Data_Struct* p_data);
void foc_3loop_update(Pi_Para_Struct* pi1, Pi_Para_Struct* pi2, Pi_Para_Struct* pi3q, Pi_Para_Struct* pi3d, Pi_Data_Struct* p_data);



// 速度环平滑启停算法

// 文件名: pid_3loops.h (结构体定义部分追加)
typedef struct
{
    float target;   // 最终目标速度（你期望达到的稳态速度）
    float output;   // 斜坡当前输出（实际喂给速度环的 target_speed）
    float accel;    // 加速度: 速度环每执行一次的增量（单位与 speed 一致/步）
} SpeedRamp_Struct;
extern SpeedRamp_Struct speed_ramp_s;

float speed_ramp_update(SpeedRamp_Struct *r);



#endif

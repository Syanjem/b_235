#ifndef _FOC_H_
#define _FOC_H_

#include "gpio.h"
#include "ATK.h"


#include "Motor_ctrl.h"


extern volatile uint8_t foc_mode;
void ADC0_1_IRQHandler(void);

void foc_standby_task(void);
void foc_working_task(void);

void atk_task(void);
void over_i_stop_task(void);

void foc_pi_task(sub_mode_control_t fcm);





void foc_debug(uint8_t f_m, float target, uint32_t d1, uint32_t d2);


void foc_feedback_update(pi_state_t* pd, angle_state_t* pa, current_state_t* pi);

extern volatile uint32_t num;
extern uint16_t dnum2;
extern uint16_t dnum1;
void foc_1loop_update(pi_param_t* pi3d, pi_param_t* pi3q, pi_state_t* p_data);
void foc_2loop_update(pi_param_t* pi2, pi_param_t* pi3q, pi_param_t* pi3d, pi_state_t* p_data);
void foc_3loop_update(pi_param_t* pi1, pi_param_t* pi2, pi_param_t* pi3q, pi_param_t* pi3d, pi_state_t* p_data);



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

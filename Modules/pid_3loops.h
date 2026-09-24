#ifndef _PID_3_LOOPS_H_
#define _PID_3_LOOPS_H_

#include "gd32f30x_libopt.h"
#include "timer.h"
#include "adc.h"
#include "spi.h"

#include "Angle_Feedback.h"
#include "Idq_Feedback.h"
#include "svpwm.h"

#include "debug.h"

typedef struct
{
    float kp;             // 比例系数
    float ki;             // 积分系数
    float integral;       // 积分累加值
    float out_max;        // 输出上限
    float out_min;        // 输出下限
    float integral_max;   // 积分限幅
} pi_param_t;

extern pi_param_t g_pi_mangle_param;   // 角度环 PI 参数
extern pi_param_t g_pi_speed_param;   // 速度环 PI 参数
extern pi_param_t g_pi_iq_param;      // q 轴电流环 PI 参数
extern pi_param_t g_pi_id_param;      // d 轴电流环 PI 参数


typedef struct
{
    float feedback_mangle;
    float feedback_speed;
    float feedback_iq;
    float feedback_id;

    float target_mangle;
    float target_speed;
    float target_iq;
    float target_id;

    float target_vq;
    float target_vd;
} pi_state_t;

extern pi_state_t g_pi_state;

void PID_Feedback_Update(pi_state_t* ps, angle_state_t* pa, current_state_t* pi);

void PID_1Loop_Target_Update(float tid, float tiq, pi_state_t* p_state);
void PID_2Loop_Target_Update(float s, pi_state_t* p_state);
void PID_3Loop_Target_Update(float ma, pi_state_t* p_state);

void pi1_mangle_loop(pi_param_t* pid1, pi_state_t* p_state);
void pi2_speed_loop(pi_param_t* pid2, pi_state_t* p_state);
void pi3_iq_loop(pi_param_t* pid3q, pi_state_t* p_state);
void pi3_id_loop(pi_param_t* pid3d, pi_state_t* p_state);
















#endif

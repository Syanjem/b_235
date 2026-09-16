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
    float Kp;           // 比例系数
    float Ki;           // 积分系数
    float integral;     // 积分累加值
    float out_max;      // 输出上限
    float out_min;      // 输出下限
    float integral_max; // 积分限幅
} Pi_Para_Struct;
extern Pi_Para_Struct pi1_mangle_para, pi2_speed_para, pi3_iq_para, pi3_id_para;


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
} Pi_Data_Struct;
extern Pi_Data_Struct pi_data_s;

void PID_Feedback_Update(Pi_Data_Struct* pd, Angle_Struct* pa, Idq_Struct* pi);

void PID_1Loop_Target_Update(float tid, float tiq, Pi_Data_Struct* p_data);
void PID_2Loop_Target_Update(float s, Pi_Data_Struct* p_data);
void PID_3Loop_Target_Update(float ma, Pi_Data_Struct* p_data);

void pi1_mangle_loop(Pi_Para_Struct* pid1, Pi_Data_Struct* p_data);
void pi2_speed_loop(Pi_Para_Struct* pid2, Pi_Data_Struct* p_data);
void pi3_iq_loop(Pi_Para_Struct* pid3q, Pi_Data_Struct* p_data);
void pi3_id_loop(Pi_Para_Struct* pid3d, Pi_Data_Struct* p_data);

void v_update(V_Struct* pv, Angle_Struct* pa, Idq_Struct* pi, Pi_Data_Struct* p_data);









void v_update_debug(V_Struct* pv, Angle_Struct* pa, Idq_Struct* pi, Pi_Data_Struct* p_data);















#endif

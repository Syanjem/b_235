#include "pid_3loops.h"

Pi_Para_Struct pi1_mangle_para = {	// 位置环就不需要了
	.Kp 			= 10.0f,
	.Ki 			= 0.1f,
	.integral 		= 0.0f,
	.out_max 		= 1.0f,		// 标幺化
	.out_min 		= -1.0f,
	.integral_max 	= 0.9f,
};

Pi_Para_Struct pi2_speed_para = {
	.Kp 			= 0.1f,
	.Ki 			= 0.01f,
	.integral 		= 0.0f,
	.out_max 		= 1.0f,		// 标幺化
	.out_min 		= -1.0f,
	.integral_max 	= 200.0f,
};

Pi_Para_Struct pi3_iq_para = {
    .Kp 			= 0.2f,         // 0.3 → 0.2, 配合减小的滤波延迟
    .Ki 			= 0.05f,        // 不变
    .integral 		= 0.0f,
    .out_max 		= 1.0f,
    .out_min 		= -1.0f,
    .integral_max 	= 20.0f,   		// Ki*integral_max = 0.05*20 = 1.0pu
};
	
Pi_Para_Struct pi3_id_para = {
    .Kp 			= 0.2f,         // 0.3 → 0.2, 配合减小的滤波延迟
    .Ki 			= 0.05f,        // 不变
    .integral 		= 0.0f,			
    .out_max 		= 1.0f,
    .out_min 		= -1.0f,
    .integral_max 	= 20.0f,   		// Ki*integral_max = 0.05*20 = 1.0pu
};

Pi_Data_Struct pi_data_s = {
	.feedback_mangle	= 0.0f,
	.feedback_speed		= 0.0f,
	.feedback_iq		= 0.0f,
	.feedback_id		= 0.0f,

	.target_mangle		= 200.0f,
	.target_speed		= 0.0f,
	.target_iq			= 0.0f,
	.target_id			= 0.0f,
	
	.target_vq			= 0.0f,
	.target_vd			= 0.0f,
};


void PID_Feedback_Update(Pi_Data_Struct* pd, Angle_Struct* pa, Idq_Struct* pi)
{
	pd->feedback_mangle = pa->angle_ma;
	pd->feedback_speed	= pa->speed;
	pd->feedback_id = pi->id;
	pd->feedback_iq = pi->iq;
}

void PID_1Loop_Target_Update(float tid, float tiq, Pi_Data_Struct* p_data)
{
	p_data->target_id = tid;
	p_data->target_iq = tiq;
}

void PID_2Loop_Target_Update(float ts, Pi_Data_Struct* p_data)
{
	p_data->target_speed = ts;
}

void PID_3Loop_Target_Update(float tma, Pi_Data_Struct* p_data)
{
	p_data->target_mangle = tma;
}


void pi1_mangle_loop(Pi_Para_Struct* pid1, Pi_Data_Struct* p_data)
{
    // 1.更新误差
    float error = (float)(p_data->target_mangle - p_data->feedback_mangle);
    
    // 2.更新积分 + 积分限幅
    pid1->integral += error;
    if (pid1->integral > pid1->integral_max) 
	{
        pid1->integral = pid1->integral_max;
    } 
	else if (pid1->integral < -pid1->integral_max) 
	{
        pid1->integral = -pid1->integral_max;
    }
    
    // 3.更新输出 + 输出限幅
    p_data->target_speed = pid1->Kp * error + pid1->Ki * pid1->integral;
    if (p_data->target_speed > pid1->out_max) 
	{
        p_data->target_speed = pid1->out_max;
    } 
	else if (p_data->target_speed < pid1->out_min) 
	{
        p_data->target_speed = pid1->out_min;
    }

}


void pi2_speed_loop(Pi_Para_Struct* pid2, Pi_Data_Struct* p_data)
{
    float error = p_data->target_speed - p_data->feedback_speed;

    // 抗积分饱和: 预判输出是否会饱和
    float pred_out = pid2->Kp * error + pid2->Ki * (pid2->integral + error);
    if (pred_out <= pid2->out_max && pred_out >= pid2->out_min)
    {
        pid2->integral += error;
        if (pid2->integral > pid2->integral_max)
            pid2->integral = pid2->integral_max;
        else if (pid2->integral < -pid2->integral_max)
            pid2->integral = -pid2->integral_max;
    }

    p_data->target_iq = pid2->Kp * error + pid2->Ki * pid2->integral;
    p_data->target_iq = CLAMP(p_data->target_iq, pid2->out_min, pid2->out_max);
    p_data->target_id = 0.0f;
}

void pi3_iq_loop(Pi_Para_Struct* pid3q, Pi_Data_Struct* p_data)
{
    // 计算误差
    float error = p_data->target_iq - p_data->feedback_iq;

    // 抗积分饱和: 预判输出是否会饱和, 饱和时不累加积分
    float pred_out = pid3q->Kp * error + pid3q->Ki * (pid3q->integral + error);
    if (pred_out <= pid3q->out_max && pred_out >= pid3q->out_min)
    {
        pid3q->integral += error;
        if (pid3q->integral > pid3q->integral_max)
            pid3q->integral = pid3q->integral_max;
        else if (pid3q->integral < -pid3q->integral_max)
            pid3q->integral = -pid3q->integral_max;
    }

    // PI输出（带限幅）
    p_data->target_vq = pid3q->Kp * error + pid3q->Ki * pid3q->integral;
    if (p_data->target_vq > pid3q->out_max)
        p_data->target_vq = pid3q->out_max;
    else if (p_data->target_vq < pid3q->out_min)
        p_data->target_vq = pid3q->out_min;
}

void pi3_id_loop(Pi_Para_Struct* pid3d, Pi_Data_Struct* p_data)
{
    // 计算误差
    float error = 0.0f - p_data->feedback_id;

    // 抗积分饱和: 预判输出是否会饱和, 饱和时不累加积分
    float pred_out = pid3d->Kp * error + pid3d->Ki * (pid3d->integral + error);
    if (pred_out <= pid3d->out_max && pred_out >= pid3d->out_min)
    {
        pid3d->integral += error;
        if (pid3d->integral > pid3d->integral_max)
            pid3d->integral = pid3d->integral_max;
        else if (pid3d->integral < -pid3d->integral_max)
            pid3d->integral = -pid3d->integral_max;
    }

    // PI输出（带限幅）
    p_data->target_vd = pid3d->Kp * error + pid3d->Ki * pid3d->integral;
    if (p_data->target_vd > pid3d->out_max)
        p_data->target_vd = pid3d->out_max;
    else if (p_data->target_vd < pid3d->out_min)
        p_data->target_vd = pid3d->out_min;
}

// pv 从 pa 获取电角度，计算三角函数
void v_update(V_Struct* pv, Angle_Struct* pa, Idq_Struct* pi, Pi_Data_Struct* p_data)
{
	pv->v_q = p_data->target_vq;	// pu
	pv->v_d = p_data->target_vd;	// pu

	// 调制比 k = sqrt(3) * |Vdq|_pu / Vbus_pu
	// vdq 是 pu, vbus 也要转 pu, 否则 k 会严重偏小
	float vdq_mag = sqrtf(pv->v_d * pv->v_d + pv->v_q * pv->v_q);
	float vbus_pu = pi->vbus_V / pi->U_Base;	// 实际 V / 基准 V = pu
	if (vbus_pu < 0.1f) vbus_pu = 0.1f;			// 兜底: 防除零
	pv->k = _SQRT3 * vdq_mag / vbus_pu;
	if (pv->k > 0.4f) pv->k = 0.4f;		// 过调制限幅

	inverse_park(pv->v_d, pv->v_q, pa->angle_ea * M_PI / 180.0f, &pv->v_alpha, &pv->v_beta);
}


void v_update_debug(V_Struct* pv, Angle_Struct* pa, Idq_Struct* pi, Pi_Data_Struct* p_data)
{
	pv->v_q = p_data->target_vq;	
	pv->v_d = p_data->target_vd;
		
	inverse_park(pv->v_d, pv->v_q, pa->angle_ea * M_PI / 180.0f, &pv->v_alpha, &pv->v_beta);
}




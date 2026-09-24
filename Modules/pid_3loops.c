#include "pid_3loops.h"

pi_param_t g_pi_mangle_param = {	// 位置环就不需要了
	.kp 			= 10.0f,
	.ki 			= 0.1f,
	.integral 		= 0.0f,
	.out_max 		= 1.0f,		// 标幺化
	.out_min 		= -1.0f,
	.integral_max 	= 0.9f,
};

pi_param_t g_pi_speed_param = {
	.kp 			= 0.1f,
	.ki 			= 0.01f,
	.integral 		= 0.0f,
	.out_max 		= 1.0f,		// 标幺化
	.out_min 		= -1.0f,
	.integral_max 	= 200.0f,
};

pi_param_t g_pi_iq_param = {
    .kp 			= 0.2f,         // 0.3 → 0.2, 配合减小的滤波延迟
    .ki 			= 0.05f,        // 不变
    .integral 		= 0.0f,
    .out_max 		= 1.0f,
    .out_min 		= -1.0f,
    .integral_max 	= 20.0f,   		// Ki*integral_max = 0.05*20 = 1.0pu
};
	
pi_param_t g_pi_id_param = {
    .kp 			= 0.2f,         // 0.3 → 0.2, 配合减小的滤波延迟
    .ki 			= 0.05f,        // 不变
    .integral 		= 0.0f,			
    .out_max 		= 1.0f,
    .out_min 		= -1.0f,
    .integral_max 	= 20.0f,   		// Ki*integral_max = 0.05*20 = 1.0pu
};

pi_state_t g_pi_state = {
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


void PID_Feedback_Update(pi_state_t* ps, angle_state_t* pa, current_state_t* pi)
{
	ps->feedback_mangle = pa->angle_mech;
	ps->feedback_speed	= pa->speed;
	ps->feedback_id = pi->i_d;
	ps->feedback_iq = pi->i_q;
}

void PID_1Loop_Target_Update(float tid, float tiq, pi_state_t* p_state)
{
	p_state->target_id = tid;
	p_state->target_iq = tiq;
}

void PID_2Loop_Target_Update(float ts, pi_state_t* p_state)
{
	p_state->target_speed = ts;
}

void PID_3Loop_Target_Update(float tma, pi_state_t* p_state)
{
	p_state->target_mangle = tma;
}


void pi1_mangle_loop(pi_param_t* pid1, pi_state_t* p_state)
{
    // 1.更新误差
    float error = (float)(p_state->target_mangle - p_state->feedback_mangle);
    
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
    p_state->target_speed = pid1->kp * error + pid1->ki * pid1->integral;
    if (p_state->target_speed > pid1->out_max) 
	{
        p_state->target_speed = pid1->out_max;
    } 
	else if (p_state->target_speed < pid1->out_min) 
	{
        p_state->target_speed = pid1->out_min;
    }

}


void pi2_speed_loop(pi_param_t* pid2, pi_state_t* p_state)
{
    float error = p_state->target_speed - p_state->feedback_speed;

    // 抗积分饱和: 预判输出是否会饱和
    float pred_out = pid2->kp * error + pid2->ki * (pid2->integral + error);
    if (pred_out <= pid2->out_max && pred_out >= pid2->out_min)
    {
        pid2->integral += error;
        if (pid2->integral > pid2->integral_max)
            pid2->integral = pid2->integral_max;
        else if (pid2->integral < -pid2->integral_max)
            pid2->integral = -pid2->integral_max;
    }

    p_state->target_iq = pid2->kp * error + pid2->ki * pid2->integral;
    p_state->target_iq = CLAMP(p_state->target_iq, pid2->out_min, pid2->out_max);
    p_state->target_id = 0.0f;
}

void pi3_iq_loop(pi_param_t* pid3q, pi_state_t* p_state)
{
    // 计算误差
    float error = p_state->target_iq - p_state->feedback_iq;

    // 抗积分饱和: 预判输出是否会饱和, 饱和时不累加积分
    float pred_out = pid3q->kp * error + pid3q->ki * (pid3q->integral + error);
    if (pred_out <= pid3q->out_max && pred_out >= pid3q->out_min)
    {
        pid3q->integral += error;
        if (pid3q->integral > pid3q->integral_max)
            pid3q->integral = pid3q->integral_max;
        else if (pid3q->integral < -pid3q->integral_max)
            pid3q->integral = -pid3q->integral_max;
    }

    // PI输出（带限幅）
    p_state->target_vq = pid3q->kp * error + pid3q->ki * pid3q->integral;
    if (p_state->target_vq > pid3q->out_max)
        p_state->target_vq = pid3q->out_max;
    else if (p_state->target_vq < pid3q->out_min)
        p_state->target_vq = pid3q->out_min;
}

void pi3_id_loop(pi_param_t* pid3d, pi_state_t* p_state)
{
    // 计算误差
    float error = 0.0f - p_state->feedback_id;

    // 抗积分饱和: 预判输出是否会饱和, 饱和时不累加积分
    float pred_out = pid3d->kp * error + pid3d->ki * (pid3d->integral + error);
    if (pred_out <= pid3d->out_max && pred_out >= pid3d->out_min)
    {
        pid3d->integral += error;
        if (pid3d->integral > pid3d->integral_max)
            pid3d->integral = pid3d->integral_max;
        else if (pid3d->integral < -pid3d->integral_max)
            pid3d->integral = -pid3d->integral_max;
    }

    // PI输出（带限幅）
    p_state->target_vd = pid3d->kp * error + pid3d->ki * pid3d->integral;
    if (p_state->target_vd > pid3d->out_max)
        p_state->target_vd = pid3d->out_max;
    else if (p_state->target_vd < pid3d->out_min)
        p_state->target_vd = pid3d->out_min;
}






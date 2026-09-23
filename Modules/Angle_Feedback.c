#include "Angle_Feedback.h"

angle_state_t g_angle = {
	
	.speed_base			= 8000.0f,		// 8000 rpm
	
//	.direction			= 1,
//	.pole_pairs 		= 4u,
	.direction			= -1,
	.pole_pairs 		= 2u,
	.angle_zero			= 185.0f,
	
	.angle_encoder		= 0.0f,
	.angle_dir_calib	= 0.0f,
	.angle_mech			= 0.0f,
	.angle_mech_prev	= 0.0f,
	.speed				= 0.0f,
	.speed_prev			= 0.0f,
	
	.angle_elec			= 0.0f,
	
	.angle_encoder_debug	= 0u,
	.angle_step_debug		= 23u,
	.angle_elec_debug		= 0u,
};


void Angle_Feedback_Update(angle_state_t* pa)
{
	// 1.更新 ma_pre
	pa->angle_mech_prev = pa->angle_mech;
	
	// 2.encoder 测量角度
	angleCs_float_fromEncoder(&(pa->angle_encoder));
	angleDf_float_fix(&(pa->angle_dir_calib), pa->direction, pa->angle_encoder);
	
	// 3.更新 ma
	float a = pa->angle_dir_calib - pa->angle_zero;
	if (a >= 0.0f)
	{
		pa->angle_mech = a;
	}
	else
	{
		pa->angle_mech = 360.0f + a;
	}

	// 首拍保护: 用真实机械角初始化前值, 消除上电时 0°->实际角度 的虚假差分
	static uint8_t first = 1u;
	if (first)
	{
		first = 0u;
		pa->angle_mech_prev = pa->angle_mech;
		pa->speed = 0.0f;
	}

	// 4.更新 ea
	pa->angle_elec = pa->pole_pairs * pa->angle_mech;
	while (pa->angle_elec >= 360.0f)
	{
		pa->angle_elec -= 360.0f;
	}
}

void Speed_Feedback_Update(angle_state_t* pa)
{
    float d = (pa->angle_mech - pa->angle_mech_prev);
    if (d >= 180.0f)        d -= 360.0f;
    else if (d <= -180.0f)  d += 360.0f;

    float speed_raw = d * 20.0f;   // 千度/秒, 原始值

    // 物理合理性钳位: 6000RPM=36千度/秒, 单拍超出即为角度毛刺(SPI误读/EMI), 截断兜底
    if (speed_raw > 40.0f)        speed_raw = 40.0f;
    else if (speed_raw < -40.0f)  speed_raw = -40.0f;

    // 一阶低通滤波: 截止频率 ≈ 速度环带宽的 1/5
    // α = 2π·fc·Ts / (1 + 2π·fc·Ts), Ts = 1/20000
    // 取 fc = 200Hz → α ≈ 0.06
    pa->speed = pa->speed * 0.98f + speed_raw * 0.02f;
}










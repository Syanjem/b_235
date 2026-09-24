#include "Foc.h"



volatile uint8_t foc_mode = 1;
volatile float foc_target_set = 0.0f;
volatile uint8_t can_stop = 0;

void ADC0_1_IRQHandler(void)
{
    if (RESET != adc_flag_get(ADC0, ADC_FLAG_EOIC)) 
    {
        adc_flag_clear(ADC0, ADC_FLAG_EOIC);	
		
		// 1.更新反馈数据
		foc_feedback_update(motorData.pi.p_data, motorData.components.p_angle, motorData.components.p_idq);	
		
		switch(motorData.state.mode)
		{
			case STATE_MODE_STANDBY:
			{
				foc_standby_task();
				break;
			}
			case STATE_MODE_WORKING:
			{
				foc_working_task();
				break;
			}
			default : break;
		}

    }
}

void foc_standby_task(void)
{
	// foc 的不同启动模式
	switch(motorData.state.start_mode)
	{
		case START_MODE_POWERUP:
		{
			motorData.state.request.switch_request = STATE_MODE_SWITCH_ON;
			motorData.state.request.target_mode = STATE_MODE_WORKING;
			break;
		}
		case START_MODE_ADC:
		{
			
			/* 零点 ≈ 2048，偏离 ±40（约 ±0.6A）才认为有真实电流跳变 */
			if(motorData.components.p_idq->i_c_sample < 1908 || motorData.components.p_idq->i_c_sample > 2188)
			{
				motorData.runtime.adc_trig_cnt++;
				if (motorData.runtime.adc_trig_cnt >= 3)   /* 连续 3 次（约150us）确认，去抖 */
				{
					motorData.state.request.switch_request = STATE_MODE_SWITCH_ON;
					motorData.state.request.target_mode = STATE_MODE_WORKING;
					motorData.runtime.adc_trig_cnt = 0;
				}
			}
			else
			{
				motorData.runtime.adc_trig_cnt = 0;         /* 一旦回到零点窗口，计数清零 */
			}
			break;		
		}
		case START_MODE_CAN:
		{
//			if (can_ok == 1)
//			{
//				motorData.state.request.switch_request = STATE_MODE_SWITCH_ON;
//				motorData.state.request.target_mode = STATE_MODE_WORKING;
//				GPIO_canWait_end();
//			}	
			break;		
		}
		case START_MODE_EXTI:
		{
			if (exti_foc_ok == 1)
			{
				motorData.state.request.switch_request = STATE_MODE_SWITCH_ON;
				motorData.state.request.target_mode = STATE_MODE_WORKING;
				exti_foc_ok = 0;
			}	
			break;		
		}
	}
}



void foc_working_task(void)
{
	
//			/* CAN 停止命令: 切回 CAN_SIGNAL 待机, 重新配 EXTI 等下次启动 */
//			if (can_stop == 1)
//			{
//				can_stop = 0;
//				motorData.state.focRunningBeginMode = FOC_RUNNING_BEGIN_MODE_CAN_SIGNAL;
//				GPIO_canWait_start();
//			}
	
	// foc pi 算法
	foc_pi_task(motorData.state.control_mode);

	v_update(motorData.components.p_v, 
				motorData.pi.p_data->target_vq,
				motorData.pi.p_data->target_vd,
				motorData.components.p_idq->vbus, 
				motorData.components.p_angle->angle_elec);
	pwm_output_update(&g_voltage, &g_pwm_duty);
	
	// 逻辑分析仪获取数据
	if(motorData.sw.atk_enable == ATK_ON)
	{
		atk_task();
	}

	// 过流停机
	over_i_stop_task();

}


// ATK
void atk_task(void)
{
	static uint16_t iq16 = 0u;
	static uint16_t id16 = 0u;
	static uint16_t spd16 = 0u;
	static uint16_t tspd16 = 0u;
	
	static uint16_t atk_num	= 0u;
	atk_num = (atk_num + 1) % 100;
	if (atk_num % 100 == 0)
	{
		// iq/id 标幺化 [-1, +1]pu, 编码到 [0, 65535]
		float iq = CLAMP(motorData.components.p_idq->i_q, -1.0f, 1.0f);
		iq16 = (uint16_t)((iq + 1.0f) * 32767.5f);
		float id = CLAMP(motorData.components.p_idq->i_d, -1.0f, 1.0f);
		id16 = (uint16_t)((id + 1.0f) * 32767.5f);

		// 速度单位是千度/秒, 范围 ±36 (6000RPM=36千度/秒), 编码到 [0, 65535]
		float spd = CLAMP(motorData.components.p_angle->speed, -40.0f, 40.0f);
		spd16 = (uint16_t)((spd + 40.0f) / 80.0f * 65535.0f);
		float tspd = CLAMP(motorData.pi.p_data->target_speed, -40.0f, 40.0f);
		tspd16 = (uint16_t)((tspd + 40.0f) / 80.0f * 65535.0f);

// 		按需选择发送: iq/id/feedback_speed/target_speed
//		spi0_ATK_16bit(iq16);
//		spi0_ATK_16bit(id16);
//		spi0_ATK_16bit(spd16);
//		spi0_ATK_16bit(tspd16);
	}
	
}

void over_i_stop_task(void)
{
	if (motorData.runtime.stop_delay_num <51000)
	{
		motorData.runtime.stop_delay_num++;
	}
	
	float q = CLAMP(motorData.components.p_idq->i_q, -1.0f, 1.0f);
	uint16_t iq_stop = (uint16_t)((q + 1.0f) * 32767.5f);
	
	if (iq_stop <= 25000 && motorData.runtime.stop_delay_num >= 50000)
	{
		motorData.runtime.iq16_up ++;	
	} 
	else 
	{ 
		motorData.runtime.iq16_up = 0; 
	}
	
	if (motorData.runtime.iq16_up >=5)
	{
		motorData.state.request.switch_request = STATE_MODE_SWITCH_ON;
		motorData.state.request.target_mode = STATE_MODE_STOPPED;
	}
}


void foc_pi_task(SUB_MODE_CONTROL fcm)
{
	switch (fcm)
	{
		case CONTROL_MODE_I:
		{
			foc_1loop_update(motorData.pi.p_id, motorData.pi.p_iq, motorData.pi.p_data); // 转矩模式
			break;
		}
		case CONTROL_MODE_SPEED:
		{		

			break;
		}
		case CONTROL_MODE_SPEED_RAMP:
		{
			foc_2loop_update(motorData.pi.p_speed, motorData.pi.p_id, motorData.pi.p_iq, motorData.pi.p_data);
			break;
		}
	}
}


void foc_debug(uint8_t f_m, float ta, uint32_t d1, uint32_t d2)
{
	foc_mode = f_m;
	if (f_m == 1u)
	{
		PID_1Loop_Target_Update(0.0f, ta, &g_pi_state);
	}
	else if (f_m == 2u)
	{
//		PID_2Loop_Target_Update(ta, &g_pi_state);
		speed_ramp_s.target = ta;
		dnum2 = d2;
	}
	else if (f_m == 3u)
	{
		PID_3Loop_Target_Update(ta, &g_pi_state);
		dnum1 = d1;
		dnum2 = d2;
	}
}

// 更新 foc 反馈数据
void foc_feedback_update(pi_state_t* ps, angle_state_t* pa, current_state_t* pi)
{	
	Angle_Feedback_Update(pa);		// 反馈角度
	Speed_Feedback_Update(pa);		// 反馈速度
	Idq_Feedback_Update(pi, pa->angle_elec);	// 反馈电流
	
	PID_Feedback_Update(ps, pa, pi);	// Pi 环反馈输入
}



volatile uint32_t num = 0;
uint16_t dnum2 = 10;
uint16_t dnum1 = 100;
void foc_1loop_update(pi_param_t* pi3d, pi_param_t* pi3q, pi_state_t* p_state)
{
	
	pi3_iq_loop(pi3q, p_state);
	pi3_id_loop(pi3d, p_state);
}


void foc_2loop_update(pi_param_t* pi2, pi_param_t* pi3q, pi_param_t* pi3d, pi_state_t* p_state)
{
	
	if (num % dnum2 == 0)
	{
        speed_ramp_update(&speed_ramp_s);              // 先更新斜坡
        g_pi_state.target_speed = speed_ramp_s.output;  // 斜坡输出作为速度目标
        pi2_speed_loop(pi2, p_state);                   // 再跑速度环
	}
//	if (num % 1 == 0)
//	{
		pi3_iq_loop(pi3q, p_state);
		pi3_id_loop(pi3d, p_state);
//	}
	num = (num+1) % dnum2;
}


void foc_3loop_update(pi_param_t* pi1, pi_param_t* pi2, pi_param_t* pi3q, pi_param_t* pi3d, pi_state_t* p_state)
{
	
	if (num % dnum1 == 0)
	{
		pi1_mangle_loop(pi1, p_state);
	}
	if (num % dnum2 == 0)
	{
		pi2_speed_loop(pi2, p_state);
	}
//	if (num % 1 == 0)
//	{
		pi3_iq_loop(pi3q, p_state);
		pi3_id_loop(pi3d, p_state);
//	}
	num = (num+1) % dnum1;
}






// 速度环平滑启停算法

// 文件名: pid_3loops.c (实现)
SpeedRamp_Struct speed_ramp_s = {
    .target  = 0.0f,
    .output  = 0.0f,
    .accel   = 0.01f, // 每步增量, 按需调整: 越大启停越猛, 越小越平滑
};

/**
 * 速度斜坡更新 (线性加减速, 支持正反转/过零)
 * 返回值: 当前斜坡输出, 可直接作为 target_speed
 *
 * 调用频率 = 速度环执行频率 (由 dnum2 分频决定)
 * 每次 pi2_speed_loop 之前调用一次即可
 */
float speed_ramp_update(SpeedRamp_Struct *r)
{
    float diff = r->target - r->output;
//	static uint16_t update_ramp = 0u;
	
    if (diff > r->accel) {
        r->output += r->accel;			// 还差很多, 加一步
    } else if (diff < -r->accel) {
        r->output -= r->accel;			// 反向还差很多, 减一步
    } else {
        r->output = r->target;			// 一步内可达, 直接到位
    }
	
	
    return r->output;
}

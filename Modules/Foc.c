#include "Foc.h"

uint16_t ma_test = 0u;

volatile uint16_t ATK_data = 0u;
volatile uint16_t atk_num = 0u;

volatile uint8_t foc_mode = 1;
volatile float foc_target_set = 0.0f;
volatile uint8_t can_stop = 0;
volatile uint16_t num_send = 0;
void ADC0_1_IRQHandler(void)
{
    if (RESET != adc_flag_get(ADC0, ADC_FLAG_EOIC)) 
    {
        adc_flag_clear(ADC0, ADC_FLAG_EOIC);	
		
		foc_task();
    }
}



void foc_task(void)
{
	// 1.更新反馈数据
	foc_feedback_update(motorData.pi.p_pidata, motorData.components.p_angle, motorData.components.p_idq);	
	
	static uint32_t p_num = 0u;
	
	// 2.foc 的不同启动模式
	switch(motorData.state.focRunningState)
	{
		case FOC_RUNNING_STATE_RUNNING_LOOP:
		{
			if (p_num <51000)
			{
				p_num++;
			}
			/* CAN 停止命令: 切回 CAN_SIGNAL 待机, 重新配 EXTI 等下次启动 */
			if (can_stop == 1)
			{
				can_stop = 0;
				motorData.state.focRunningBeginMode = FOC_RUNNING_BEGIN_MODE_CAN_SIGNAL;
				GPIO_canWait_start();
			}
			
			// foc pi 算法
			foc_pi_task(motorData.state.focRunningControlMode);

			v_update(motorData.components.p_v, 
						motorData.pi.p_pidata->target_vq,
						motorData.pi.p_pidata->target_vd,
						motorData.components.p_idq->vbus_V, 
						motorData.components.p_angle->angle_ea);
			pwm_output_update(&v_s, &abc_s);
			
			break;		
		}

		case FOC_RUNNING_STATE_ADC_DETECTION_LOOP:
		{
			static uint8_t adc_trig_cnt = 0;
			
			/* 零点 ≈ 2048，偏离 ±40（约 ±0.6A）才认为有真实电流跳变 */
			if(motorData.components.p_idq->ic_shot < 1908 || motorData.components.p_idq->ic_shot > 2188)
			{
				adc_trig_cnt++;
				if (adc_trig_cnt >= 3)   /* 连续 3 次（约150us）确认，去抖 */
				{
					motorData.state.focSwitchState = FOC_SWITCH_STATE_ADC_DETECTION_OUT;
					adc_trig_cnt = 0;
				}
			}
			else
			{
				adc_trig_cnt = 0;         /* 一旦回到零点窗口，计数清零 */
			}
			break;		
		}

		case FOC_RUNNING_STATE_CAN_SIGNAL_LOOP:
		{
//			if (can_ok == 1)
//			{
//				motorData.state.FOC_RUNNING_BEGIN_MODE = FOC_RUNNING_BEGIN_MODE_POWER_UP;
//				GPIO_canWait_end();
//			}	
			break;		
		}
		
		case FOC_RUNNING_STATE_GPIO_EXTI_LOOP:
		{
			if (exti_foc_ok == 1)
			{
				motorData.state.focSwitchState = FOC_SWITCH_STATE_GPIO_EXTI_OUT;
			}	
			break;		
		}

	}
	
	// ATK
	static uint16_t iq16 = 0u;
	
		if (atk_num % 100 == 0)
		{
			// iq/id 标幺化 [-1, +1]pu, 编码到 [0, 65535]
			float iq = CLAMP(motorData.components.p_idq->iq, -1.0f, 1.0f);
			iq16 = (uint16_t)((iq + 1.0f) * 32767.5f);
			float id = CLAMP(motorData.components.p_idq->id, -1.0f, 1.0f);
			uint16_t id16 = (uint16_t)((id + 1.0f) * 32767.5f);

			// 速度单位是千度/秒, 范围 ±36 (6000RPM=36千度/秒), 编码到 [0, 65535]
			float spd = CLAMP(motorData.components.p_angle->speed, -40.0f, 40.0f);
			uint16_t spd16 = (uint16_t)((spd + 40.0f) / 80.0f * 65535.0f);
			float tspd = CLAMP(motorData.pi.p_pidata->target_speed, -40.0f, 40.0f);
			uint16_t tspd16 = (uint16_t)((tspd + 40.0f) / 80.0f * 65535.0f);

			// 按需选择发送: iq/id/feedback_speed/target_speed
//						spi0_ATK_16bit(iq16);
//						spi0_ATK_16bit(id16);
//						spi0_ATK_16bit(spd16);
//						spi0_ATK_16bit(tspd16);
		}
		atk_num = (atk_num + 1) % 100;
	
	static uint8_t iq16_up = 0u;
	if (iq16 <= 25000 && p_num >= 50000)
	{
		iq16_up ++;	
	}
	if (iq16_up >=5)
	{
		motorData.state.focSwitchState = FOC_SWITCH_STATE_STOP;
	}

}


void foc_begin_mode_choose_task(void)
{
	switch (motorData.state.focRunningBeginMode)
	{
		case FOC_RUNNING_BEGIN_MODE_POWER_UP: 
		{
			motorData.state.focInState = FOC_IN_STATE_OFF;
			motorData.state.focRunningState = FOC_RUNNING_STATE_RUNNING_LOOP;
			break;
		}
		case FOC_RUNNING_BEGIN_MODE_ADC_DETECTION:
		{
			motorData.state.focInState = FOC_IN_STATE_ADC_DETECTION_IN;
			motorData.state.focRunningState = FOC_RUNNING_STATE_ADC_DETECTION_LOOP;
			GPIO_adcBackDetect_in();
			break;
		}
		case FOC_RUNNING_BEGIN_MODE_CAN_SIGNAL:
		{
			motorData.state.focInState = FOC_IN_STATE_CAN_SIGNAL_IN;
			motorData.state.focRunningState = FOC_RUNNING_STATE_CAN_SIGNAL_LOOP;
			GPIO_canWait_start();
			break;
		}
		case FOC_RUNNING_BEGIN_MODE_GPIO_EXTI:
		{	
			motorData.state.focInState = FOC_IN_STATE_GPIO_EXTI_IN;
			motorData.state.focRunningState = FOC_RUNNING_STATE_GPIO_EXTI_LOOP;
			GPIO_extiWait_start();
			break;
		}
	}
}

void foc_switch_mode_task(void)
{
	switch (motorData.state.focSwitchState)
	{
		case FOC_SWITCH_STATE_ADC_DETECTION_OUT: 
		{
			motorData.state.focSwitchState		= FOC_SWITCH_STATE_OFF;
			__disable_irq();
			motorData.state.focRunningState = FOC_RUNNING_STATE_RUNNING_LOOP;
			GPIO_adcBackDetect_out();
			__enable_irq();	
			break;			
		}
		case FOC_SWITCH_STATE_CAN_SIGNAL_OUT:	break;
		case FOC_SWITCH_STATE_GPIO_EXTI_OUT:
		{
			motorData.state.focSwitchState		= FOC_SWITCH_STATE_OFF;
			__disable_irq();
			motorData.state.focRunningState = FOC_RUNNING_STATE_RUNNING_LOOP;
			GPIO_extiWait_end();
			__enable_irq();	
			break;		
		}
		
		case FOC_SWITCH_STATE_STOP:
		{
			motorData.state.focSwitchState		= FOC_SWITCH_STATE_OFF;
			__disable_irq();
			GPIO_adcBackDetect_in();
			adc_disable(ADC0);
			timer_disable(TIMER0);
			break;
		}
		case FOC_SWITCH_STATE_OFF:	break;
	}
}



void foc_pi_task(FOC_RUNNING_CONTROL_MODE fcm)
{
	switch (fcm)
	{
		case FOC_RUNNING_CONTROL_MODE_I:
		{
			foc_1loop_update(motorData.pi.pi3_id_para, motorData.pi.pi3_iq_para, motorData.pi.p_pidata); // 转矩模式
			break;
		}
		case FOC_RUNNING_CONTROL_MODE_SPEED:
		{		

			break;
		}
		case FOC_RUNNING_CONTROL_MODE_SPEED_RAMP:
		{
			foc_2loop_update(motorData.pi.pi2_speed_para, motorData.pi.pi3_id_para, motorData.pi.pi3_iq_para, motorData.pi.p_pidata);
			break;
		}
	}
}





void foc_debug(uint8_t f_m, float ta, uint32_t d1, uint32_t d2)
{
	foc_mode = f_m;
	if (f_m == 1u)
	{
		PID_1Loop_Target_Update(0.0f, ta, &pi_data_s);
	}
	else if (f_m == 2u)
	{
//		PID_2Loop_Target_Update(ta, &pi_data_s);
		speed_ramp_s.target = ta;
		dnum2 = d2;
	}
	else if (f_m == 3u)
	{
		PID_3Loop_Target_Update(ta, &pi_data_s);
		dnum1 = d1;
		dnum2 = d2;
	}
}

// 更新 foc 反馈数据
void foc_feedback_update(Pi_Data_Struct* pd, Angle_Struct* pa, Idq_Struct* pi)
{	
	Angle_Feedback_Update(pa);		// 反馈角度
	Speed_Feedback_Update(pa);		// 反馈速度
	Idq_Feedback_Update(pi, pa->angle_ea);	// 反馈电流
	
	PID_Feedback_Update(pd, pa, pi);	// Pi 环反馈输入
}



volatile uint32_t num = 0;
uint16_t dnum2 = 10;
uint16_t dnum1 = 100;
void foc_1loop_update(Pi_Para_Struct* pi3d, Pi_Para_Struct* pi3q, Pi_Data_Struct* p_data)
{
	
	pi3_iq_loop(pi3q, p_data);
	pi3_id_loop(pi3d, p_data);
}


void foc_2loop_update(Pi_Para_Struct* pi2, Pi_Para_Struct* pi3q, Pi_Para_Struct* pi3d, Pi_Data_Struct* p_data)
{
	
	if (num % dnum2 == 0)
	{
        speed_ramp_update(&speed_ramp_s);              // 先更新斜坡
        pi_data_s.target_speed = speed_ramp_s.output;  // 斜坡输出作为速度目标
        pi2_speed_loop(pi2, p_data);                   // 再跑速度环
	}
//	if (num % 1 == 0)
//	{
		pi3_iq_loop(pi3q, p_data);
		pi3_id_loop(pi3d, p_data);
//	}
	num = (num+1) % dnum2;
}


void foc_3loop_update(Pi_Para_Struct* pi1, Pi_Para_Struct* pi2, Pi_Para_Struct* pi3q, Pi_Para_Struct* pi3d, Pi_Data_Struct* p_data)
{
	
	if (num % dnum1 == 0)
	{
		pi1_mangle_loop(pi1, p_data);
	}
	if (num % dnum2 == 0)
	{
		pi2_speed_loop(pi2, p_data);
	}
//	if (num % 1 == 0)
//	{
		pi3_iq_loop(pi3q, p_data);
		pi3_id_loop(pi3d, p_data);
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

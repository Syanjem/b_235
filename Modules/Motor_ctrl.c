#include "Motor_ctrl.h"

MOTOR_DATA motorData = {
	.state = {
		.mode					= STATE_MODE_STANDBY,
		.request = {
			.switch_request 	= STATE_MODE_SWITCH_ON,
			.target_mode		= STATE_MODE_STANDBY,
		},
//		.fault
		.start_mode				= START_MODE_ADC,
		.control_mode			= CONTROL_MODE_SPEED_RAMP,
	},
	
	.components	= {
		.p_angle  = &g_angle,
		.p_idq	  = &g_current,
		.p_v	  = &v_s,
		.p_abcpwm = &abc_s,
	},
	
	.pi = {
		.p_data 	= &pi_data_s,
		.p_iq		= &pi3_iq_para,
		.p_id		= &pi3_id_para,
		.p_speed	= &pi2_speed_para,
		.p_mangle	= &pi1_mangle_para,
	},

	.runtime = {
		.stop_delay_num = 0u,
		.iq16_up	  = 0u,
		.adc_trig_cnt = 0u,
	},

	.sw = {
		.atk_enable = ATK_ON,
	}
};


void motor_state_mode_config_task(void)
{
	if(motorData.state.request.switch_request == STATE_MODE_SWITCH_ON)
	{
		motorData.state.request.switch_request = STATE_MODE_SWITCH_OFF;
		switch(motorData.state.request.target_mode)
		{
			case STATE_MODE_STANDBY: 
			{	
				motorData.state.mode = STATE_MODE_STANDBY;
				motor_standby_config(); 
				break; 
			}
			case STATE_MODE_WORKING: 
			{	
				motorData.state.mode = STATE_MODE_WORKING;
				motor_working_config(); 
				break; 
			}
			case STATE_MODE_STOPPED: 
			{	
				motorData.state.mode = STATE_MODE_STOPPED;
				motor_stopped_config(); 
				break; 
			}
//			case STATE_MODE_CALIBRATING: 
//			{	
//				motorData.state.mode = STATE_MODE_CALIBRATING;
//				motor_calibrating_config(); 
//				break; 
//			}
			default: { break; }
		}
	}
}


void motor_standby_config(void)
{
	switch (motorData.state.start_mode)
	{
		case START_MODE_POWERUP: 
		{
			break;
		}
		case START_MODE_ADC:
		{
			GPIO_adcBackDetect_in();
			break;
		}
		case START_MODE_CAN:
		{
//			GPIO_canWait_start();
			break;
		}
		case START_MODE_EXTI:
		{	
			GPIO_extiWait_start();
			break;
		}
	}
}


void motor_working_config(void)
{
	switch (motorData.state.start_mode)
	{
		case START_MODE_POWERUP: 
		{
			break;
		}
		case START_MODE_ADC: 
		{
			__disable_irq();
			GPIO_adcBackDetect_out();
			__enable_irq();	
			break;			
		}
		case START_MODE_CAN:	break;
		case START_MODE_EXTI:
		{
			__disable_irq();
			GPIO_extiWait_end();
			__enable_irq();	
			break;		
		}
	}
}


void motor_stopped_config(void)
{
	__disable_irq();
	GPIO_adcBackDetect_in();
	adc_disable(ADC0);
	timer_disable(TIMER0);
}











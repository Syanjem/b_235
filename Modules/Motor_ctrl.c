#include "Motor_ctrl.h"

motor_data_t g_motor_data = {
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
		.p_v	  = &g_voltage,
		.p_abcpwm = &g_pwm_duty,
	},
	
	.pi = {
		.p_data 	= &g_pi_state,
		.p_iq		= &g_pi_iq_param,
		.p_id		= &g_pi_id_param,
		.p_speed	= &g_pi_speed_param,
		.p_mangle	= &g_pi_mangle_param,
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
	if(g_motor_data.state.request.switch_request == STATE_MODE_SWITCH_ON)
	{
		g_motor_data.state.request.switch_request = STATE_MODE_SWITCH_OFF;
		switch(g_motor_data.state.request.target_mode)
		{
			case STATE_MODE_STANDBY: 
			{	
				g_motor_data.state.mode = STATE_MODE_STANDBY;
				motor_standby_config(); 
				break; 
			}
			case STATE_MODE_WORKING: 
			{	
				g_motor_data.state.mode = STATE_MODE_WORKING;
				motor_working_config(); 
				break; 
			}
			case STATE_MODE_STOPPED: 
			{	
				g_motor_data.state.mode = STATE_MODE_STOPPED;
				motor_stopped_config(); 
				break; 
			}
//			case STATE_MODE_CALIBRATING: 
//			{	
//				g_motor_data.state.mode = STATE_MODE_CALIBRATING;
//				motor_calibrating_config(); 
//				break; 
//			}
			default: { break; }
		}
	}
}


void motor_standby_config(void)
{
	switch (g_motor_data.state.start_mode)
	{
		case START_MODE_POWERUP: 
		{
			break;
		}
		case START_MODE_ADC:
		{
			gpio_adc_back_detect_in();
			break;
		}
		case START_MODE_CAN:
		{
//			gpio_can_wait_start();
			break;
		}
		case START_MODE_EXTI:
		{	
			gpio_exti_wait_start();
			break;
		}
	}
}


void motor_working_config(void)
{
	switch (g_motor_data.state.start_mode)
	{
		case START_MODE_POWERUP: 
		{
			break;
		}
		case START_MODE_ADC: 
		{
			__disable_irq();
			gpio_adc_back_detect_out();
			__enable_irq();	
			break;			
		}
		case START_MODE_CAN:	break;
		case START_MODE_EXTI:
		{
			__disable_irq();
			gpio_exti_wait_end();
			__enable_irq();	
			break;		
		}
	}
}


void motor_stopped_config(void)
{
	__disable_irq();
	gpio_adc_back_detect_in();
	adc_disable(ADC0);
	timer_disable(TIMER0);
}











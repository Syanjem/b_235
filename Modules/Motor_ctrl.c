#include "Motor_ctrl.h"

MOTOR_DATA motorData = {
	.state = {
		.stateMode					= STATE_MODE_STANDBY,
		.standby_subMode_start		= START_MODE_ADC,
		.working_subMode_control	= CONTROL_MODE_SPEED_RAMP,
		.stateFlag = {
			.STATE_MODE_SWITCH_FLAG	= STATE_MODE_SWITCH_ON,
			.ATK_FLAG				= ATK_ON,
			.STATE_MODE_FLAG		= STATE_MODE_STANDBY,
		},
	},
	
	.components	= {
		.p_angle  = &angle_s,
		.p_idq	  = &idq_s,
		.p_v	  = &v_s,
		.p_abcpwm = &abc_s,
	},
	
	.pi = {
		.p_pidata 			= &pi_data_s,
		.pi3_iq_para		= &pi3_iq_para,
		.pi3_id_para		= &pi3_id_para,
		.pi2_speed_para		= &pi2_speed_para,
		.pi1_mangle_para	= &pi1_mangle_para,
	},
};


void motor_state_mode_config_task(void)
{
	if(motorData.state.stateFlag.STATE_MODE_SWITCH_FLAG == STATE_MODE_SWITCH_ON)
	{
		motorData.state.stateFlag.STATE_MODE_SWITCH_FLAG = STATE_MODE_SWITCH_OFF;
		switch(motorData.state.stateFlag.STATE_MODE_FLAG)
		{
			case STATE_MODE_STANDBY: 
			{	
				motorData.state.stateMode = STATE_MODE_STANDBY;
				motor_standby_config(); 
				break; 
			}
			case STATE_MODE_WORKING: 
			{	
				motorData.state.stateMode = STATE_MODE_WORKING;
				motor_working_config(); 
				break; 
			}
			case STATE_MODE_STOPPED: 
			{	
				motorData.state.stateMode = STATE_MODE_STOPPED;
				motor_stopped_config(); 
				break; 
			}
//			case STATE_MODE_CALIBRATING: 
//			{	
//				motorData.state.stateMode = STATE_MODE_CALIBRATING;
//				motor_calibrating_config(); 
//				break; 
//			}
			default: { break; }
		}
	}
}


void motor_standby_config(void)
{
	switch (motorData.state.standby_subMode_start)
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
	switch (motorData.state.standby_subMode_start)
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











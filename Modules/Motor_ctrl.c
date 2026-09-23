#include "Motor_ctrl.h"




MOTOR_DATA motorData = {
	.state = {
		.stateMode					= STATE_MODE_STANDBY,
		.standby_subMode_start		= START_MODE_ADC,
		.working_subMode_control	= CONTROL_MODE_SPEED_RAMP,
		.stateFlag = {
			.START_ADC_FLAG = START_ADC_OFF,
			.START_CAN_FLAG = START_CAN_OFF,
			.START_EXTI_FLAG	= START_EXTI_OFF,
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













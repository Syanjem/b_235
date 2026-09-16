#include "Motor_ctrl.h"




MOTOR_DATA motorData = {
	.components	= {
		.p_angle  = &angle_s,
		.p_idq	  = &idq_s,
		.p_v	  = &v_s,
		.p_abcpwm = &abc_s,
	},
	
	.state = {
		.state_mode			= STATE_MODE_FocRunning,
		.foc_begin_mode		= FOC_BEGIN_MODE_POWER_UP,
		.foc_control_mode	= FOC_CONTROL_MODE_SPEED_RAMP,
	},
	
	.pi = {
		.p_pidata 			= &pi_data_s,
		.pi3_iq_para		= &pi3_iq_para,
		.pi3_id_para		= &pi3_id_para,
		.pi2_speed_para		= &pi2_speed_para,
		.pi1_mangle_para	= &pi1_mangle_para,
	},
};





void MotorStateTask(MOTOR_DATA *motor)
{
//    switch (motor->state.state_mode)
//    {
//    case STATE_MODE_Init:	// 空闲模式
//        PID_clear(&motor->IqPID);
//        PID_clear(&motor->IdPID);
//        PID_clear(&motor->VelPID);
//        PID_clear(&motor->PosPID);
//        FOC_reset(motor->components.foc);
//        Foc_Pwm_LowSides();
//        motor->state.State_Mode = STATE_MODE_DETECTING;
//        motor->state.Sub_State  = CURRENT_CALIBRATING;
//        break;
//    case STATE_MODE_JiaoZhun: // 电机矫正模式
//        MotorInitializeTask(motor);
//        break;
//    case STATE_MODE_JianCe: // 运行模式
//        MotorControlTask(motor);
//        break;
//    case STATE_MODE_Foc: // 守护模式
//        PID_clear(&motor->IqPID);
//        PID_clear(&motor->IdPID);
//        PID_clear(&motor->VelPID);
//        PID_clear(&motor->PosPID);
//        FOC_reset(motor->components.foc);
//        Foc_Pwm_LowSides();
//        break;
//    case STATE_MODE_Fault: // 电机矫正模式
//        MotorInitializeTask(motor);
//        break;
//    case STATE_MODE_DEBUG: // 电机矫正模式
//        MotorInitializeTask(motor);
//        break;
//    }
}










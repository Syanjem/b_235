#include "gd32f30x.h"
#include "systick.h"
#include <stdio.h>
#include "main.h"

#include "BSP.h"


#include "Foc.h"
#include "JiaoZhun.h"

#include "general_def.h"


int main(void)
{
	bsp_sys_clock_init();
	bsp_timer_trigger_adc_detect_config();	
	bsp_connect_init();		
	
	// 1. 初始化
	g_motor_data.state.mode 				= STATE_MODE_STANDBY;
	g_motor_data.state.start_mode		 	= START_MODE_ADC;
	g_motor_data.state.control_mode		= CONTROL_MODE_SPEED_RAMP;
	
	// 2. 进入待机模式
	g_motor_data.state.request.switch_request = STATE_MODE_SWITCH_ON;
	g_motor_data.state.request.target_mode	   = STATE_MODE_STANDBY;
	
	switch (g_motor_data.state.control_mode)
	{
		case CONTROL_MODE_I:
		{
			foc_debug(1, -0.04f, 100, 10);
			break;
		}
		case CONTROL_MODE_SPEED:
		{
			break;
		}
		case CONTROL_MODE_SPEED_RAMP:
		{
			foc_debug(2, -2.5f, 100, 10);	// 千度/秒
			break;
		}
	}
	delay_1ms(1000);
	
	
//	2. 校准（ma_zero = 94.0f）
//	angle_zero_float_get((&angle_s);
//	delay_1ms(1000);
	
	bsp_timer_trigger_adc_detect_enable();	// 使能 adc0, timer0
	
	
	while(1)
	{
		
		motor_state_mode_config_task();
		

	}
}


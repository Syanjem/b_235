#include "gd32f30x.h"
#include "systick.h"
#include <stdio.h>
#include "main.h"

#include "BSP.h"


#include "Foc.h"
#include "JiaoZhun.h"

#include "general_def.h"

#include "debug.h"


int main(void)
{
	BSP_SysClock_Init();
	BSP_timerTrigger_adcDetect_Config();
	BSP_Connect_Init();		
	
	// 1. 初始化
	motorData.state.stateMode = STATE_MODE_RUNNING;
	motorData.state.focRunningBeginMode = FOC_RUNNING_BEGIN_MODE_ADC_DETECTION;
	motorData.state.focRunningControlMode = FOC_RUNNING_CONTROL_MODE_SPEED_RAMP;
	
	switch (motorData.state.focRunningControlMode)
	{
		case FOC_RUNNING_CONTROL_MODE_I:
		{
			foc_debug(1, -0.04f, 100, 10);
			break;
		}
		case FOC_RUNNING_CONTROL_MODE_SPEED:
		{
			break;
		}
		case FOC_RUNNING_CONTROL_MODE_SPEED_RAMP:
		{
			foc_debug(2, -2.5f, 100, 10);	// 千度/秒
			break;
		}
	}
	delay_1ms(2000);
	
	
//	2. 校准（ma_zero = 94.0f）
//	angleZero_float_get((&angle_s);
//	delay_1ms(1000);
	
	switch (motorData.state.focRunningBeginMode)
	{
		case FOC_RUNNING_BEGIN_MODE_POWER_UP: 
		{
			break;
		}
		case FOC_RUNNING_BEGIN_MODE_ADC_DETECTION:
		{
			GPIO_adcBackRead_start();
			break;
		}
		case FOC_RUNNING_BEGIN_MODE_CAN_SIGNAL:
		{
			GPIO_canWait_start();
			break;
		}
		case FOC_RUNNING_BEGIN_MODE_GPIO_EXTI:
		{	
			GPIO_extiWait_start();
			break;
		}
	}

	
	BSP_timerTrigger_adcDetect_Enable();
	
	
//	测试零点偏移
//	uint16_t acs = 0u;
//	uint16_t adfz = 0u;
	
	
	while(1)
	{
		
		if (stop == 1u)
		{
			__disable_irq();
			GPIO_adcBackRead_start();
			adc_disable(ADC0);
			timer_disable(TIMER0);
			stop = 2u;
		}
		
		
//		测试零点偏移
//		delay_1ms(10);
//		angleCs_uint16_t_fromEncoder(&acs);
//		spi0_ATK_16bit((uint16_t)(360u - acs));
		
		
//		debug_open23_sendEa_dirFix(23);	// 递减
		
		
//		a++;
//		if(a == 100000 && foc_mode == 1)
//		{
//			a = 0;
//			
//			can_message_transmit(CAN0, &debug_code[1]);
//			while(can_transmit_states(CAN0, 0) == CAN_TRANSMIT_PENDING);
//			foc_mode = 2;
//		}
//		else if(a == 100000 && foc_mode == 2)
//		{
//			a = 0;
//			
//			can_message_transmit(CAN0, &debug_code[0]);
//			while(can_transmit_states(CAN0, 0) == CAN_TRANSMIT_PENDING);
//			foc_mode = 1;
//		}
//		
//		if (can0_rx_full_flag)
//		{
//			can0_rx_full_flag = 0;
//			
//			if (rx_int_msg[0].rx_data[0] == 0x01)
//			{
//				foc_mode = 1;
//			}
//			else if (rx_int_msg[0].rx_data[0] == 0x02)
//			{
//				foc_mode = 2;
//			}
//		}

	}
}


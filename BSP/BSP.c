#include "BSP.h"

void BSP_SysClock_Init(void)
{
	RCU_Init();
    SysTick_Init();	
}

// disable
void BSP_timerTrigger_adcDetect_Config(void)	
{
	ADC0_Init();
	TIMER0_Init();
}

void BSP_timerTrigger_adcDetect_Enable(void)
{
	adc_enable(ADC0);	
	delay_1ms(1);
	adc_calibration_enable(ADC0);
	timer_enable(TIMER0);
}

void BSP_Connect_Init(void)
{
	SPI0_Init();
	I2C0_Init();
//	CAN_Init();		
}

















// rcu，systick
// timer0，adc0，spi0
// MB1601B，FPC
// JJKG
void BSP_Init_Task(void)
{
	// 时钟



	// 温控加热
	MB1601B_Init();
	FPC_Init();
	
	// 接近开关
	JJKG_Init();	
	
	// 磁编通信、三相驱动、电流反馈
	// spi0:  	 2MHz
	// timer0: 120MHz
	// adc0: 	20MHz
	SPI0_Init();
	TIMER0_Init();
//	ADC0_Init();
}


void BSP_Monitor_Mode_InitConfig_Task(void)
{
	// 1.adc0 中断获取反馈电流
    timer_primary_output_config(TIMER0, DISABLE);	// 失能 timer0 的三相输出
	GPIO_abcBackDetect_Config();							// 维持三相下管导通
	adc_enable(ADC0);	
	delay_1ms(1);
	adc_calibration_enable(ADC0);	
	
	// 2.传感器周期发送温度使能
	MB1601B_Write_Scratchpad();
}





#include "BSP.h"

void bsp_sys_clock_init(void)
{
	rcu_init();
    SysTick_Init();	
}

// disable
void bsp_timer_trigger_adc_detect_config(void)	
{
	adc0_init();
	timer0_init();
}

void bsp_timer_trigger_adc_detect_enable(void)
{
	adc_enable(ADC0);	
	delay_1ms(1);
	adc_calibration_enable(ADC0);
	timer_enable(TIMER0);
}

void bsp_connect_init(void)
{
	spi0_init();
	i2c0_init();
//	can0_init();		
}




// rcu，systick
// timer0，adc0，spi0
// MB1601B，FPC
// JJKG
void bsp_init_task(void)
{
	// 时钟



	// 温控加热
	MB1601B_Init();
	fpc_init();
	
	// 接近开关
	jjkg_init();	
	
	// 磁编通信、三相驱动、电流反馈
	// spi0:  	 2MHz
	// timer0: 120MHz
	// adc0: 	20MHz
	spi0_init();
	timer0_init();
//	adc0_init();
}


void bsp_monitor_mode_init_config_task(void)
{
	// 1.adc0 中断获取反馈电流
    timer_primary_output_config(TIMER0, DISABLE);	// 失能 timer0 的三相输出
	gpio_abc_back_detect_config();							// 维持三相下管导通
	adc_enable(ADC0);	
	delay_1ms(1);
	adc_calibration_enable(ADC0);	
	
	// 2.传感器周期发送温度使能
	MB1601B_Write_Scratchpad();
}





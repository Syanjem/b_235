#include "adc.h"

// PV: PA2, ADC01_IN2
// Ic: PB0, ADC01_IN8
// Ib: PB1, ADC01_IN9

void ADC0_Init(void)
{
	rcu_periph_clock_enable(RCU_ADC0);
	rcu_periph_clock_enable(RCU_GPIOB);
	rcu_periph_clock_enable(RCU_GPIOA);									// PA2 (PV 母线电压采样)
	rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV6);	// 20MHz
	gpio_init(GPIOB, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_0);	// PB0 = Ic
	gpio_init(GPIOB, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_1);	// PB1 = Ib
	gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_2);	// PA2 = PV (母线电压)
	adc_deinit(ADC0);	
	
	adc_mode_config(ADC_MODE_FREE);
	
	adc_special_function_config(ADC0, ADC_SCAN_MODE, ENABLE);			
	adc_special_function_config(ADC0, ADC_CONTINUOUS_MODE, DISABLE);	// 非连续模式
	adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);				// 右对齐
	adc_resolution_config(ADC0, ADC_RESOLUTION_12B);					// 12 分辨率
	
	adc_channel_length_config(ADC0, ADC_INSERTED_CHANNEL, 3);	
	adc_inserted_channel_config(ADC0, 0, ADC_CHANNEL_8, ADC_SAMPLETIME_7POINT5);	// 20 = 7.5 + 12.5
	adc_inserted_channel_config(ADC0, 1, ADC_CHANNEL_9, ADC_SAMPLETIME_7POINT5);	// 20 = 7.5 + 12.5
	adc_inserted_channel_config(ADC0, 2, ADC_CHANNEL_2, ADC_SAMPLETIME_7POINT5);	// 20 = 7.5 + 12.5

	adc_external_trigger_source_config(ADC0, ADC_INSERTED_CHANNEL, ADC0_1_EXTTRIG_INSERTED_T0_TRGO);	// TIMER0 的 TRGO
	adc_external_trigger_config(ADC0, ADC_INSERTED_CHANNEL, ENABLE);	
	
	adc_interrupt_enable(ADC0, ADC_INT_EOIC);
	nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
    nvic_irq_enable(ADC0_1_IRQn, 0, 0);
	
	adc_disable(ADC0);
}







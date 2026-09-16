#include "gpio.h"

// Foc �� adc ��ⴥ��ģʽ
void GPIO_abcDown_Config(void)
{
	
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10);
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);

	gpio_bit_reset(GPIOA, GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10);
	gpio_bit_set(GPIOB, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
}


void GPIO_adcBackRead_start(void)
{
	timer_primary_output_config(TIMER0, DISABLE);
	GPIO_abcDown_Config();
}


void GPIO_adcBackRead_end(void)
{
	TIMER0_Init();
	timer_enable(TIMER0);
}



void GPIO_canWait_Config(void)
{
	rcu_periph_clock_enable(RCU_GPIOB);
	
    gpio_init(GPIOB, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_8 | GPIO_PIN_9);
	
	gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOB, GPIO_PIN_SOURCE_8 | GPIO_PIN_SOURCE_9);
	
	exti_init(EXTI_8, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
	exti_init(EXTI_9, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
	exti_interrupt_enable(EXTI_8);
	exti_interrupt_enable(EXTI_9);
	nvic_irq_enable(EXTI5_9_IRQn, 0, 2);
}

void GPIO_canWait_start(void)
{
	i2c_disable(I2C0);
	GPIO_canWait_Config();
}

void GPIO_canWait_end(void)
{
	i2c_enable(I2C0);
}





void GPIO_extiWait_Config(void)
{
	rcu_periph_clock_enable(RCU_GPIOB);
	
    gpio_init(GPIOB, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_8 | GPIO_PIN_9);
	
	gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOB, GPIO_PIN_SOURCE_8 | GPIO_PIN_SOURCE_9);
	
	exti_init(EXTI_8, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
	exti_init(EXTI_9, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
	exti_interrupt_enable(EXTI_8);
	exti_interrupt_enable(EXTI_9);
	nvic_irq_enable(EXTI5_9_IRQn, 0, 2);
}

void GPIO_extiWait_start(void)
{
    timer_disable(TIMER0);                          // 关计数器, 彻底停 PWM
    timer_primary_output_config(TIMER0, DISABLE);   // 关主输出
    GPIO_abcDown_Config();                           // 三相引脚切 OUT_PP, 高侧关低侧通
    i2c_disable(I2C0);
    GPIO_extiWait_Config();  
}

void GPIO_extiWait_end(void)
{
    exti_interrupt_disable(EXTI_8 | EXTI_9);   // 关 EXTI
    i2c_enable(I2C0);
    TIMER0_Init();
    timer_enable(TIMER0);
}

volatile uint8_t exti_foc_ok = 0;
void EXTI5_9_IRQHandler(void)
{
	if ((RESET != exti_interrupt_flag_get(EXTI_8)) || (RESET != exti_interrupt_flag_get(EXTI_9)))
	{
		exti_interrupt_flag_clear(EXTI_8);
		exti_interrupt_flag_clear(EXTI_9);
		
		
		exti_foc_ok = 1;
	}
}





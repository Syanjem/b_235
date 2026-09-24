#include "gpio.h"

// adc 检测模式的 gpio 配置
void gpio_abc_back_detect_config(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10);
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);

	gpio_bit_reset(GPIOA, GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10);
	gpio_bit_set(GPIOB, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
}


void gpio_adc_back_detect_in(void)
{
	timer_enable(TIMER0);
	timer_primary_output_config(TIMER0, DISABLE);
	gpio_abc_back_detect_config();
}


void gpio_adc_back_detect_out(void)
{
	timer0_init();
	timer_enable(TIMER0);
}



void gpio_can_wait_config(void)
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

void gpio_can_wait_start(void)
{
	i2c_disable(I2C0);
	gpio_can_wait_config();
}

void gpio_can_wait_end(void)
{
	i2c_enable(I2C0);
}


void gpio_exti_wait_config(void)
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

void gpio_exti_wait_start(void)
{
    timer_disable(TIMER0);                          // 关计数器, 彻底停 PWM
    timer_primary_output_config(TIMER0, DISABLE);   // 关主输出
    gpio_abc_back_detect_config();                  // 三相引脚切 OUT_PP, 高侧关低侧通
    i2c_disable(I2C0);
    gpio_exti_wait_config();  
}

void gpio_exti_wait_end(void)
{
    exti_interrupt_disable(EXTI_8);   // 关 EXTI
	exti_interrupt_disable(EXTI_9);
    i2c_enable(I2C0);
    timer0_init();
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





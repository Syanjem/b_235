#include "JJKG.h"

// JJKG1: PB3
// JJKG2: PA15
void jjkg_init(void)
{
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOA);

    gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_3);
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_15);
	
    exti_deinit();
	exti_init(EXTI_3, EXTI_INTERRUPT, EXTI_TRIG_RISING);
	exti_init(EXTI_15, EXTI_INTERRUPT, EXTI_TRIG_RISING);

	exti_interrupt_enable(EXTI_3);
	exti_interrupt_enable(EXTI_15);

	nvic_irq_enable(EXTI3_IRQn, 0, 0);
	nvic_irq_enable(EXTI10_15_IRQn, 0, 0); 
}

// JJKG1
void EXTI3_IRQHandler(void)
{
    if(exti_interrupt_flag_get(EXTI_3) != RESET) 
    {
        exti_interrupt_flag_clear(EXTI_3); 
		
		
    }
}

// JJKG2
void EXTI15_10_IRQHandler(void)
{
    if(exti_interrupt_flag_get(EXTI_15) != RESET) 
    {
        exti_interrupt_flag_clear(EXTI_15); 
		
		
		
    }
}



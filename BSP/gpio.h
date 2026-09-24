#ifndef _GPIO_H_
#define _GPIO_H_

#include "gd32f30x_libopt.h"
#include "timer.h"


void gpio_abc_back_detect_config(void);
void gpio_adc_back_detect_in(void);
void gpio_adc_back_detect_out(void);


void gpio_can_wait_config(void);
void gpio_can_wait_start(void);
void gpio_can_wait_end(void);



void gpio_exti_wait_config(void);
void gpio_exti_wait_start(void);
void gpio_exti_wait_end(void);
extern volatile uint8_t exti_foc_ok;
void EXTI5_9_IRQHandler(void);

#endif

#ifndef _GPIO_H_
#define _GPIO_H_

#include "gd32f30x_libopt.h"
#include "timer.h"


void GPIO_abcDown_Config(void);
void GPIO_adcBackRead_start(void);
void GPIO_adcBackRead_end(void);


void GPIO_canWait_Config(void);
void GPIO_canWait_start(void);
void GPIO_canWait_end(void);



void GPIO_extiWait_Config(void);
void GPIO_extiWait_start(void);
void GPIO_extiWait_end(void);
extern volatile uint8_t exti_foc_ok;
void EXTI5_9_IRQHandler(void);

#endif

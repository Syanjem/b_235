#ifndef _FPC_JR_H_
#define _FPC_JR_H_

#include "gd32f30x_libopt.h"
#include "systick.h"

#include "temperature_1wire.h"

#define FPC_POTR		GPIOB
#define FPC_PIN 		GPIO_PIN_5

#define FPC_ENABLE()	gpio_bit_set(GPIOB, GPIO_PIN_5)
#define FPC_DISABLE()	gpio_bit_reset(GPIOB, GPIO_PIN_5)


void FPC_Init(void);


#endif

#ifndef _BSP_H_
#define _BSP_H_

#include "systick.h"
#include "rcu.h"

#include "timer.h"
#include "adc.h"

#include "spi.h"
#include "i2c.h"
#include "can.h"

#include "temperature_1wire.h"
#include "FPC_jr.h"
#include "JJKG.h"

#include "gpio.h"



void BSP_SysClock_Init(void);
void BSP_timerTrigger_adcDetect_Config(void);
void BSP_timerTrigger_adcDetect_Enable(void);
void BSP_Connect_Init(void);



void BSP_Init_Task(void);
void BSP_Monitor_Mode_InitConfig_Task(void);







#endif

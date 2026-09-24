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



void bsp_sys_clock_init(void);
void bsp_timer_trigger_adc_detect_config(void);
void bsp_timer_trigger_adc_detect_enable(void);
void bsp_connect_init(void);



void bsp_init_task(void);
void bsp_monitor_mode_init_config_task(void);







#endif

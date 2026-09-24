#ifndef _I2C_H_
#define _I2C_H_

#include "gd32f30x_libopt.h"
#include "systick.h"


void i2c0_init(void);


void i2c0_sendData_toATK_Debug(uint8_t data);
void i2c0_sendData16bits_toATK_Debug(uint16_t data);





#endif

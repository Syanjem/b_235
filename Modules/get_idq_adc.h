#ifndef _GET_IDQ_ADC_H_
#define _GET_IDQ_ADC_H_

#include "gd32f30x_libopt.h"
#include "spi.h"
#include "timer.h"
#include "adc.h"
#include "get_motor_angle.h"

#define PI			3.14159265359f
#define SQRT_1_3	0.577350269f

typedef struct
{
	int16_t ia;
	int16_t ib;
	int16_t ic;
	float cos_angle_J;
	float sin_angle_J;
	float id;
	float iq;
} I_Typedef;

extern I_Typedef i_typedef;

void i_typedef_init(I_Typedef* pi);

void get_iac_adc(I_Typedef* pi);
void get_iac_set_idq(I_Typedef* pi);





#endif

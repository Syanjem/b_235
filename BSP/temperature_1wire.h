#ifndef _TEMPERATURE_1_WIRE_H_
#define _TEMPERATURE_1_WIRE_H_

#include "gd32f30x_libopt.h"
#include "systick.h"

#define MB1601B_POTR		GPIOB
#define MB1601B_PIN 		GPIO_PIN_4

// 设置引脚为推挽输出模式
#define MB1601B_MODE_OUT() do { \
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4); \
} while(0)

// 设置引脚为上拉输入模式
#define MB1601B_MODE_IN()  do { \
    gpio_init(GPIOB, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_4); \
} while(0)

// 引脚输出宏（高/低）
#define MB1601B_OUT_1()		gpio_bit_set(GPIOB, GPIO_PIN_4)
#define MB1601B_OUT_0()		gpio_bit_reset(GPIOB, GPIO_PIN_4)

// 读取引脚电平
#define MB1601B_IN_READ()	gpio_input_bit_get(GPIOB, GPIO_PIN_4)

extern float temperature;

void MB1601B_Init(void);


void MB1601B_Write_Scratchpad(void);
uint8_t MB1601B_GetTemperature(float *temp);


uint8_t MB1601B_Reset(void);
void MB1601B_Write_Byte(uint8_t data);
uint8_t MB1601B_Read_Byte(void);









#endif

#ifndef _SPI_H_
#define _SPI_H_

#include "gd32f30x_libopt.h"

void SPI0_Init(void);

// 磁编码器（encoder）
// CSN2 : PA4
#define SPI_CSN1_encoder_SELECT()   gpio_bit_reset(GPIOA, GPIO_PIN_4)
#define SPI_CSN1_encoder_DESELECT() gpio_bit_set(GPIOA, GPIO_PIN_4)

// 逻辑分析仪（ATK）
// CSN1 : PA3
#define SPI_CSN2_ATK_SELECT()   gpio_bit_reset(GPIOA, GPIO_PIN_3)
#define SPI_CSN2_ATK_DESELECT() gpio_bit_set(GPIOA, GPIO_PIN_3)

// 上位机（UP）
// CSN1 : PA3
#define SPI_CSN2_up_SELECT()   gpio_bit_reset(GPIOA, GPIO_PIN_3)
#define SPI_CSN2_up_DESELECT() gpio_bit_set(GPIOA, GPIO_PIN_3)

uint16_t spi_readwrite_16bit(uint32_t spi_periph, uint16_t data);
void spi0_up_16bit(uint16_t data);
void spi0_up_32bit(uint16_t data1, uint16_t data2);

#endif



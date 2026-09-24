#ifndef _ENCODER_SPI_H_
#define _ENCODER_SPI_H_

#include "general_def.h"

#include "spi.h"

// ========== 读命令宏 ==========
// 读命令：0x80
#define	SPI_MT_READ_CMD1	((uint16_t)0x83FF)
#define	SPI_MT_READ_CMD2	((uint16_t)0x84FF)

void angle_cs_float_from_encoder(float* a_cs);
void angleCs_uint16_t_fromEncoder(uint16_t* a_cs);









#endif

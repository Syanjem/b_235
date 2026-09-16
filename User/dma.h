#ifndef _DMA_H_
#define _DMA_H_

#include "gd32f30x_libopt.h"

#include "per_dev.h"

extern dma_parameter_struct	dma_initTypeDef;

void DMA_Init(void);
void DMA0_SPI0_Config(uint32_t TX, uint32_t RX);
















#endif

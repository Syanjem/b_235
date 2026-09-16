#ifndef _IRQ_H_
#define _IRQ_H_

#include "gd32f30x.h"

#include "per_dev.h"

 extern  uint8_t dma_tx_done;
 extern  uint8_t dma_rx_done;

void DMA0_Channel1_IRQHandler(void);
void DMA0_Channel2_IRQHandler(void);







#endif

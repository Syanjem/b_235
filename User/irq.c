#include "irq.h"

// 全局标志
 uint8_t dma_tx_done = 0;
 uint8_t dma_rx_done = 0;

void DMA0_Channel1_IRQHandler(void)  // RX通道
{
    // 全完成中断
    if (RESET != dma_flag_get(DMA0, DMA_CH1, DMA_FLAG_FTF)) 
    {
        dma_flag_clear(DMA0, DMA_CH1, DMA_FLAG_FTF);
        dma_rx_done = 1;
        dma_channel_disable(DMA0, DMA_CH1);
        
        // 只有TX也完成时，才切换状态
        if (dma_tx_done) 
        {
            dma_tx_done = 0;
            dma_rx_done = 0;
			
			// 释放所有 CS
			SPI_NSS1_DESELECT();
			SPI_NSS2_DESELECT();
            
            if ((spi_state_1 == STATE_NSS1) && (spi_state_2 == STATE_BUSY))
			{
				spi_state_1 = STATE_NSS2;
				spi_state_2 = STATE_FREE;
			}
            else if ((spi_state_1 == STATE_NSS2) && (spi_state_2 == STATE_BUSY))
			{
				spi_state_1 = STATE_NSS1;
				spi_state_2 = STATE_FREE;
			}
        }
    }
}

void DMA0_Channel2_IRQHandler(void)  // TX通道
{
    // 全完成中断
    if (RESET != dma_flag_get(DMA0, DMA_CH2, DMA_FLAG_FTF)) 
    {
        dma_flag_clear(DMA0, DMA_CH2, DMA_FLAG_FTF);
        dma_tx_done = 1;
        dma_channel_disable(DMA0, DMA_CH2);
        
        // 只有RX也完成时，才切换状态
        if (dma_rx_done) 
        {
            dma_tx_done = 0;
            dma_rx_done = 0;
			
			// 释放所有 CS
			SPI_NSS1_DESELECT();
			SPI_NSS2_DESELECT();
			
            if ((spi_state_1 == STATE_NSS1) && (spi_state_2 == STATE_BUSY))
			{
				spi_state_1 = STATE_NSS2;
				spi_state_2 = STATE_FREE;
			}
            else if ((spi_state_1 == STATE_NSS2) && (spi_state_2 == STATE_BUSY))
			{
				spi_state_1 = STATE_NSS1;
				spi_state_2 = STATE_FREE;
			}
        }
    }
}












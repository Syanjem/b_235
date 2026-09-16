#include "dma.h"

dma_parameter_struct dma_tx_initTypeDef;
dma_parameter_struct dma_rx_initTypeDef;

void DMA_Init(void)
{
    rcu_periph_clock_enable(RCU_DMA0);
    
    // ========== CH2: SPI0 TX ==========
    dma_channel_disable(DMA0, DMA_CH2);
    dma_struct_para_init(&dma_tx_initTypeDef);
    dma_tx_initTypeDef.periph_addr  = (uint32_t)(&SPI_DATA(SPI0));
    dma_tx_initTypeDef.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;
    dma_tx_initTypeDef.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;   // 外设地址固定（SPI_DATA 寄存器）
    dma_tx_initTypeDef.memory_width = DMA_MEMORY_WIDTH_16BIT;
    dma_tx_initTypeDef.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;     // 内存地址递增
    dma_tx_initTypeDef.direction    = DMA_MEMORY_TO_PERIPHERAL;
    dma_tx_initTypeDef.number       = 3;                               // 固定 3 个数据
    dma_tx_initTypeDef.priority     = DMA_PRIORITY_HIGH;
    dma_init(DMA0, DMA_CH2, &dma_tx_initTypeDef);
    
    dma_circulation_disable(DMA0, DMA_CH2);
    dma_memory_to_memory_disable(DMA0, DMA_CH2);
    dma_interrupt_enable(DMA0, DMA_CH2, DMA_INT_FTF | DMA_INT_ERR);   // 只使能完成中断和错误中断
    
    // ========== CH1: SPI0 RX ==========
    dma_channel_disable(DMA0, DMA_CH1);
    dma_struct_para_init(&dma_rx_initTypeDef);
    dma_rx_initTypeDef.periph_addr  = (uint32_t)(&SPI_DATA(SPI0));
    dma_rx_initTypeDef.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;
    dma_rx_initTypeDef.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;   // 外设地址固定
    dma_rx_initTypeDef.memory_width = DMA_MEMORY_WIDTH_16BIT;
    dma_rx_initTypeDef.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;     // 内存地址递增
    dma_rx_initTypeDef.direction    = DMA_PERIPHERAL_TO_MEMORY;
    dma_rx_initTypeDef.number       = 3;                               // 固定 3 个数据
    dma_rx_initTypeDef.priority     = DMA_PRIORITY_MEDIUM;
    dma_init(DMA0, DMA_CH1, &dma_rx_initTypeDef);
    
    dma_circulation_disable(DMA0, DMA_CH1);
    dma_memory_to_memory_disable(DMA0, DMA_CH1);
    dma_interrupt_enable(DMA0, DMA_CH1, DMA_INT_FTF | DMA_INT_ERR);   // 只使能完成中断和错误中断
}



void DMA0_SPI0_Config(uint32_t tx_addr, uint32_t rx_addr)
{
    // ========== 配置 TX (CH2) ==========
    dma_channel_disable(DMA0, DMA_CH2);
    dma_tx_initTypeDef.memory_addr = tx_addr;
    dma_init(DMA0, DMA_CH2, &dma_tx_initTypeDef);
    
    // ========== 配置 RX (CH1) ==========
    dma_channel_disable(DMA0, DMA_CH1);
    dma_rx_initTypeDef.memory_addr = rx_addr;
    dma_init(DMA0, DMA_CH1, &dma_rx_initTypeDef);
}

















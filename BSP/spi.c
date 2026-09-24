#include "spi.h"

// SPI0 初始化
// SPI0 与磁编码器通信，与上位机/调试机通信
void spi0_init(void)
{
	rcu_periph_clock_enable(RCU_SPI0);
	rcu_periph_clock_enable(RCU_GPIOA);
	
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3);	// PA3 - NSS2	
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4);	// PA4 - NSS1
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5);	// PA5 - SCK，复用推挽输出
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7);	// PA7 - MOSI，复用推挽输出
    gpio_init(GPIOA, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_6);		// PA6 - MISO，上拉输入

    // 软件 NSS 引脚	
    gpio_bit_set(GPIOA, GPIO_PIN_3);  // 初始拉高，表示未选中
    gpio_bit_set(GPIOA, GPIO_PIN_4);  // 初始拉高，表示未选中
	
	spi_i2s_deinit(SPI0);
	spi_parameter_struct	spi_initTypeDef;
	spi_struct_para_init(&spi_initTypeDef);
		spi_initTypeDef.	prescale				= SPI_PSC_16;				// 60 / 16 = 4MHz
		spi_initTypeDef.	device_mode 			= SPI_MASTER;				// 主机模式
		spi_initTypeDef.	trans_mode				= SPI_TRANSMODE_FULLDUPLEX;	// 双向全双工
		spi_initTypeDef.	nss						= SPI_NSS_SOFT;				// 软件控制 NSS
		spi_initTypeDef.	frame_size				= SPI_FRAMESIZE_16BIT;		// 16 bit
		spi_initTypeDef.	clock_polarity_phase	= SPI_CK_PL_HIGH_PH_2EDGE;	// 模式3（磁编要求）
		spi_initTypeDef.	endian					= SPI_ENDIAN_MSB;			// 大端模式（磁编要求）
	spi_init(SPI0, &spi_initTypeDef);
	
	// 阻塞控制
	spi_dma_disable(SPI0,  SPI_DMA_RECEIVE | SPI_DMA_TRANSMIT);			
	spi_i2s_interrupt_disable(SPI0, SPI_I2S_INT_TBE | SPI_I2S_INT_RBNE);	
	
	// 使能 spi
	spi_enable(SPI0);
}


// spi 读写函数
uint16_t spi_readwrite_16bit(uint32_t spi_periph, uint16_t data)
{
//	__disable_irq();	在 adc0 的中断服务函数中调用
    while (spi_i2s_flag_get(spi_periph, SPI_FLAG_TBE) == RESET);
    spi_i2s_data_transmit(spi_periph, data);
    while (spi_i2s_flag_get(spi_periph, SPI_FLAG_RBNE) == RESET);
//	__enable_irq();
    return spi_i2s_data_receive(spi_periph);
}


void spi0_up_16bit(uint16_t data)
{
	SPI_CSN2_up_SELECT();
	SPI_CSN1_encoder_DESELECT();
	spi_readwrite_16bit(SPI0, data);
	SPI_CSN2_up_DESELECT();
}


void spi0_up_32bit(uint16_t data1, uint16_t data2)
{
	SPI_CSN2_up_SELECT();
	SPI_CSN1_encoder_DESELECT();
	spi_readwrite_16bit(SPI0, data1);
	spi_readwrite_16bit(SPI0, data2);
	SPI_CSN2_up_DESELECT();
}


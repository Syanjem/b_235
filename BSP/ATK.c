#include "ATK.h"



void spi0_ATK_16bit(uint16_t data)
{
	SPI_CSN2_ATK_SELECT();
	SPI_CSN1_encoder_DESELECT();
	spi_readwrite_16bit(SPI0, data);
	SPI_CSN2_ATK_DESELECT();
}


void spi0_ATK_32bit(uint16_t data1, uint16_t data2)
{
	SPI_CSN2_ATK_SELECT();
	SPI_CSN1_encoder_DESELECT();
	spi_readwrite_16bit(SPI0, data1);
	spi_readwrite_16bit(SPI0, data2);
	SPI_CSN2_ATK_DESELECT();
}






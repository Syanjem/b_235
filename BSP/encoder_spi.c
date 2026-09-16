#include "encoder_spi.h"


// 获取 ma_raw
void angleCs_float_fromEncoder(float* a_cs)
{
	// 开启 SPI 片选
    SPI_CSN1_encoder_SELECT();	// 选择 encoder 片选
	
	// 获取原始机械角度
	uint8_t data_adr_0x03 = spi_readwrite_16bit(SPI0, 0x83FF);
	uint8_t data_adr_0x04 = spi_readwrite_16bit(SPI0, 0x84FF);
	uint16_t data_fixed	= (uint16_t)((data_adr_0x03 << 6) | (data_adr_0x04 >> 2));

	*a_cs	= (float)data_fixed * 360.0f * ONE_BY_ENCODER;
	if (*a_cs < 0.0f || *a_cs > 360.0f)
	{
		*a_cs = 0.0f;
	}
	
	// 关闭片选
    SPI_CSN1_encoder_DESELECT();
}


void angleCs_uint16_t_fromEncoder(uint16_t* a_cs)
{
	// 开启 SPI 片选
    SPI_CSN1_encoder_SELECT();	// 选择 encoder 片选
	
	// 获取原始机械角度
	uint8_t data_adr_0x03 = spi_readwrite_16bit(SPI0, 0x83FF);
	uint8_t data_adr_0x04 = spi_readwrite_16bit(SPI0, 0x84FF);
	uint16_t data_fixed	= (uint16_t)((data_adr_0x03 << 6) | (data_adr_0x04 >> 2));

	*a_cs	= (uint16_t)((float)data_fixed * 360.0f * ONE_BY_ENCODER);
	if (*a_cs > 360u)
	{
		*a_cs = 0u;
	}
	
	// 关闭片选
    SPI_CSN1_encoder_DESELECT();
}



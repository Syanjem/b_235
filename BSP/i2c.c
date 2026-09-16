#include "i2c.h"


/* 逻辑分析仪调试用固定从机地址: 0xAA(二进制10101010易于识别), 左移1位后由库填R/W */
#define I2C0_DEBUG_SLAVE_ADDR		(0xAAu)


void I2C0_Init(void)
{
	// I2C0
	// PB8: I2C0_SCL(REMAP)
	// PB9: I2C0_SDA(REMAP)
	rcu_periph_clock_enable(RCU_I2C0);
	rcu_periph_clock_enable(RCU_GPIOB);
	rcu_periph_clock_enable(RCU_AF);
	
	gpio_pin_remap_config(GPIO_I2C0_REMAP, ENABLE);
	
	gpio_init(GPIOB, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_8);	// SCL
	gpio_init(GPIOB, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_9);	// SDA
	
	i2c_deinit(I2C0);
    i2c_clock_config(I2C0, 100000, I2C_DTCY_2); 									// 配置时钟为100kHz标准模式
    i2c_mode_addr_config(I2C0, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, 0x03);		// 配置为I2C模式，7位地址，自身地址设为0x03（该地址在主机模式下可不关注）
    i2c_ack_config(I2C0, I2C_ACK_ENABLE);    										// 使能应答
   
	i2c_enable(I2C0);	// 使能I2C0外设	
	
	delay_1ms(10);
}


void i2c0_sendData_toATK_Debug(uint8_t data)
{
	uint32_t to;

	/* 1) 生成 START */
	i2c_start_on_bus(I2C0);
	to = get_ms_ticks();
	while(i2c_flag_get(I2C0, I2C_FLAG_SBSEND) == RESET)
	{
		if (get_ms_ticks() - to > 10u) return;		/* 超时退出, 避免死等 */
	}

	/* 2) 发从机地址 + 写 (最低位R/W由库填0) */
	i2c_master_addressing(I2C0, I2C0_DEBUG_SLAVE_ADDR << 1, I2C_TRANSMITTER);
	to = get_ms_ticks();
	while(i2c_flag_get(I2C0, I2C_FLAG_ADDSEND) == RESET)
	{
		if (get_ms_ticks() - to > 10u) goto _stop;	/* 无应答则直接发STOP收尾 */
	}
	i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);			/* 必须清, 否则下一次假命中跳过地址阶段 */

	/* 3) 发数据字节 */
	i2c_data_transmit(I2C0, data);
	to = get_ms_ticks();
	while(i2c_flag_get(I2C0, I2C_FLAG_BTC) == RESET)
	{
		if (get_ms_ticks() - to > 10u) goto _stop;
	}

_stop:
	/* 4) 生成 STOP */
	i2c_stop_on_bus(I2C0);
}

void i2c0_sendData16bits_toATK_Debug(uint16_t data)
{
	uint32_t to;

	/* 1) 生成 START */
	i2c_start_on_bus(I2C0);
	to = get_ms_ticks();
	while(i2c_flag_get(I2C0, I2C_FLAG_SBSEND) == RESET)
	{
		if (get_ms_ticks() - to > 10u) return;
	}

	/* 2) 发从机地址 + 写 */
	i2c_master_addressing(I2C0, I2C0_DEBUG_SLAVE_ADDR << 1, I2C_TRANSMITTER);
	to = get_ms_ticks();
	while(i2c_flag_get(I2C0, I2C_FLAG_ADDSEND) == RESET)
	{
		if (get_ms_ticks() - to > 10u) goto _stop;
	}
	i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);

	/* 3) 发高字节 */
	i2c_data_transmit(I2C0, (uint8_t)(data >> 8));
	to = get_ms_ticks();
	while(i2c_flag_get(I2C0, I2C_FLAG_TBE) == RESET)
	{
		if (get_ms_ticks() - to > 10u) goto _stop;
	}

	/* 4) 发低字节 (改用TBE: 缓冲空即可写下一个, 流水线效率更高) */
	i2c_data_transmit(I2C0, (uint8_t)(data & 0x00FF));
	to = get_ms_ticks();
	while(i2c_flag_get(I2C0, I2C_FLAG_BTC) == RESET)
	{
		if (get_ms_ticks() - to > 10u) goto _stop;
	}

_stop:
	/* 5) 生成 STOP */
	i2c_stop_on_bus(I2C0);
}

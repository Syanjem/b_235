#include "temperature_1wire.h"

void MB1601B_Init(void)
{
    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_init(MB1601B_POTR, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, MB1601B_PIN);
}

/* 单总线协议
 * 1根通信线：PB4
 * 复位-存在：主机拉低后释放，转为输入模式，从机拉低
 * 读/写字节，低位在前
 * 写时隙：主机拉低，写0/写1
 * 读时隙：主机拉低，转为输入模式，读0/读1
 */

// 配置：0x0D	000 011 01	每秒2次，重复性默认高
void MB1601B_Write_Scratchpad(void)
{
	if(!MB1601B_Reset())
	{
		MB1601B_Write_Byte(0xCC);	// Skip ROM
		MB1601B_Write_Byte(0x4E);	// Write Scratchpad
		
		MB1601B_Write_Byte(0x50);	// 默认值，不使用
		MB1601B_Write_Byte(0x00);	// 默认值，不使用
		MB1601B_Write_Byte(0x0D);	// 配置寄存器：000 011 01，每秒2次，重复性默认高
	}
	
	// 保存到 EEPROM (可选)
	if(!MB1601B_Reset())
	{
		MB1601B_Write_Byte(0xCC);
		MB1601B_Write_Byte(0x48);  // Copy Page0
		delay_1ms(50);
	}
}


float temperature = 0.0f;

// 转换并读取温度
uint8_t MB1601B_GetTemperature(float *temp)
{
    // 如果配置了MPS，芯片会自动更新温度值
    // 只需读取暂存器，无需发送0x44
    
    if (MB1601B_Reset()) 
	{
        return 1;
    }
	
    MB1601B_Write_Byte(0xCC);
    MB1601B_Write_Byte(0xBE);
    
    uint8_t lt = MB1601B_Read_Byte();
    uint8_t ht = MB1601B_Read_Byte();
    uint16_t raw = (uint16_t)(lt | (ht << 8));
    
    *temp = 40.0f + (raw / 256.0f);
	
    return 0;
}


// 复位
uint8_t MB1601B_Reset(void)
{
    uint8_t status;
    
    // 1. 发送复位脉冲（不关中断）
    MB1601B_MODE_OUT();
    MB1601B_OUT_0();
    delay_us(500);
    MB1601B_OUT_1();
    delay_us(30);
    
    // 2. 关键检测阶段（关中断）
    __disable_irq();
    MB1601B_MODE_IN();
    
    int timeCount = 0;
    while (timeCount < 240 && MB1601B_IN_READ())
    {
        delay_us(1);
        timeCount++;
    }
    
    // 立即判断检测结果（在中断恢复前）
    status = (timeCount >= 240) ? 1 : 0;  // 1:失败, 0:成功
    
    __enable_irq();   // 恢复中断
    
    // 3. 补足剩余时间（检测到设备时，才需要补足）
    if (status == 0 && timeCount < 240)
    {
        delay_us(240 - timeCount);
    }
    
    return status;
}


// 写字节
void MB1601B_Write_Byte(uint8_t data)
{
    MB1601B_MODE_OUT();
    
    for (int i = 0; i < 8; i++)
    {
        // 每个时隙的操作需要连续执行
        __disable_irq();
        
        if (data & 0x01)
        {
            MB1601B_OUT_0();
            delay_us(2);
            MB1601B_OUT_1();
            delay_us(60);
        }
        else
        {
            MB1601B_OUT_0();
            delay_us(60);
            MB1601B_OUT_1();
            delay_us(2);
        }
        
        __enable_irq();   // 每个时隙结束后恢复中断
        
        data >>= 1;
    }
}

// 读字节
uint8_t MB1601B_Read_Byte(void)
{
    uint8_t data = 0;
    
    for (int i = 0; i < 8; i++)
    {
        __disable_irq();   // 关中断保护时隙
        
        MB1601B_MODE_OUT();
        MB1601B_OUT_0();
        delay_us(2);
        MB1601B_MODE_IN();
        delay_us(8);
        
        if (MB1601B_IN_READ())
        {
            data |= (0x01 << i);
        }
        
        delay_us(50);  // 总时间 ~60us
        
        __enable_irq();  // 时隙结束恢复中断
    }
    
    return data;
}




















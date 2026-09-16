功能：
 1. 角度采集，电流采集，Foc运算，电压反变换，输出 PWM 驱动
 2. 温度采集，加热控制，接近采集
 3. CAN 通信

APB1:60MHz
	TIMER1 TIMER2
	CAN0
	I2C0

APB2:120MHz
	TIMER0
	SPI0
	ADC01
	GPIOA-G

理想：
	电流环：20kHz（同PWM）
	速度环：2kHz
	位置环：200Hz

SPI0 - 磁编 - 位置，速度
	SPI通信：2个16位，9.5us（100kHz，应该可以支持速度和位置环）
	
TIMER2 - ADC01 - 电流
	采样频率：20kHz（同PWM）








TIMER 和 ADC 驱动三相 ABC 和测量反馈电流的协作：
 1. TIMER0 的 CH210 分别驱动三相 ABC，模式为中央向上计数，pwm0（先高后低）
 2. TIMER0 的 rc 配置为 1，使得 TIMER0 的定时器【仅在计数下溢时产生 update】（高低高的 PWM 的起点）
 3. TIMER0 的 update 作为 TIMER2 的从模式触发源，TIMER2 的 cpp3 作为 ADC0 的触发源
 4. TIMER0 的周期为 3000，占空比范围为 [900, 2400]
 5. TIMER2 的周期为 3000，ccr 取 1300，实现【在三相输出的 PWM 的下管均导通时触发 ADC 测量】

	TIMER0: 0 高 900 2400 低 3600 5100 高 6000(触发) 高 6900 8400 低 9600 11100 高 12000(触发)
	TIMER2: 0(复位)    2600(触发)         0(复位)			   2600(触发)          0(复位)

 6. ADC0_IN8_PB0_Ic，ADC1_IN9_PB1_Ia
 7. ADC0 和 ADC1 设置为规则组同步触发模式
 8. TIMER 的低通时间为：900 / 120 = 7.5 us，ADC 的转换时间为：(28.5 + 12.5) / 20 = 2.5 us





思路1：
1. 主循环：获取,处理反馈，更新目标和输出(条件)；
2. 在中断里进行 pid 运算，加入更新判断；

主循环：
 1. 获取角度，处理获得反馈机械角度(校)、电角度
 2. 更新角度环反馈机械角度、速度环反馈速度
 3. 获取电流iac，处理获得反馈电流idq
 4. 更新电流环反馈电流


文件：
	Angle_Feedback.c
	Idq_Feedback.c



获取反馈电流：
1. 初始化 
（ia, ic）timer2 - adc - dma【icb_dma_buffer2】
	ic: icb_dma_buffer2 0-16
	ia: icb_dma_buffer2 17-32
（ia, ic）


















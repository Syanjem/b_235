001：6 个磁场扇区
		A
		|
		|
	   / \
      /   \
	 B     C

	A	B	C
1	1	0	0	
2	1	0	1	
3	0	0	1	
4	0	1	1	
5	0	1	0	
6	1	1	0	

扇区01：12			时序图：12					占空比：A > C > B
	A	B	C			A  0111110  1111100			0 = B + (T - A)
1	1	0	0			B  0001000 	1000000			1 = A - C
2	1	0	1			C  0011100	1110000			2 = C - B
 
扇区02：23			时序图：32					占空比：C > A > B
	A	B	C			A  0011100	1110000			0 = B + (T - C)
2	1	0	1			B  0001000	1000000			3 = C - A
3	0	0	1			C  0111110 	1111100			2 = A - B
		
扇区03：34			时序图：34					占空比：C > B > A  
	A	B	C			A  0001000	1000000			0 = A + (T - C)
3	0	0	1			B  0011100	1110000			3 = C - B
4	0	1	1			C  0111110	1111100			4 = B - A

扇区04：45			时序图：54					占空比：B > C > A
	A	B	C			A  0001000	1000000			0 = A + (T - B)
4	0	1	1			B  0111110	1111100			5 = B - C
5	0	1	0			C  0011100	1110000			4 = C - A

扇区05：56			时序图：56					占空比：B > A > C
	A	B	C			A  0011100	1110000			0 = C + (T - B)
5	0	1	0			B  0111110	1111100			5 = B - A
6	1	1	0			C  0001000	1000000			6 = A - C

扇区06：61			时序图：16					占空比：A > B > C
	A	B	C			A  0111110	1111100			0 = C + (T - A)
6	1	1	0			B  0011100	1110000			1 = A - B
1	1	0	0			C  0001000	1000000			6 = B - C



002：程序分层
时钟配置
外设配置
基本功能实现


003：进度
7-8：TIMER 配置




1.校准使得机械角度0点对齐电角度0点						1	get_motor_angle
2.获取校准后的机械角度									1	get_motor_angle
3.通过ADC获取iac，并转换成iab，进而转换成idq				1	get_idq_adc
4.角度环：输入目标角度、反馈校准后机械角度；输出目标速度	1	foc_pi
5.速度环：输入目标速度、反馈速度；输出目标iq				1	foc_pi
6.电流环：输入目标idq、反馈idq；输出目标vdq				1	foc_pi
7.将目标vdq，转换成对应的arrABC							1	vdq_arrabc
8.修改arrABC，输出PWM									1	vdq_arrabc



所需函数/数据一览:
	get_idq_h:
		i_typedef	iabc,cossin,idq
		get_idq(i_typedef*)
		get_iac_adc(i_typedef*)
	get_motor_h:
		Angle_Struct	p,mangle_d,z,f,eangle,step	// 角度结构体
		set_arrabc_eangle(uint16_t)
		motor_to_eangle(Angle_Struct*)
		eangle_addNum_step_Angle_S(Angle_Struct*)
		uint16_t Arr_ABC_360[361][4];
		set_direct_mangle(Angle_Struct*)
		set_zero_mangle(Angle_Struct*)
		set_fixed_mangle(Angle_Struct*)
	foc_pi.h
		pi123_para	target,feedback,target
		pic123
		void foc(float ta);
		void output_vdq(Voltage_DQ* pv, I_Struct* pi, PI3_Idq_para* p3);
		void pi1_mangle_loop(PI1_MAngle_para* p1, PI_Controller_Struct* pc1);
		void pi2_speed_loop(PI2_Speed_para* p2, PI_Controller_Struct* pc2);
		void pi3_id_loop(PI3_Idq_para* p3d, PI_Controller_Struct* pc3d);
		void pi3_iq_loop(PI3_Idq_para* p3q, PI_Controller_Struct* pc3);


		void PI_Init(PI_Controller_Struct *pid, float Kp, float Ki, 
					 float out_max, float out_min);
		void All_Controllers_Init(void);
		float PI_Calculate(PI_Controller_Struct *pid, float target, float feedback);
	vdq_arrabc.h
		Voltage_DQ
		Duty_abc
		void vdq_get_arrabc(const Voltage_DQ *pV, Duty_ABC *pD);
		int8_t valphabeta_get_sector(float v_alpha, float v_beta);
		void valphabeta_get_tabc(int sector, float v_alpha, float v_beta, float k,
								 uint16_t T, uint16_t *ta, uint16_t *tb, uint16_t *tc);
		void arrabc_init(Duty_ABC* pd);
		void set_arrabc(Duty_ABC* pd);




8-4：无电流环版本
1.校准使得机械角度0点对齐电角度0点						1	get_motor_angle
2.获取校准后的机械角度									1	get_motor_angle
3.通过ADC获取iac，并转换成iab，进而转换成idq				1	get_idq_adc
4.角度环：输入目标角度、反馈校准后机械角度；输出目标速度	1	foc_pi
5.速度环：输入目标速度、反馈速度；输出目标vq				1	foc_pi
7.将目标vdq，转换成对应的arrABC							1	vdq_arrabc
8.修改arrABC，输出PWM									1	vdq_arrabc




8-7 功能
1.自动拉拽识别			负载电流采样
2.转速自动匹配			FOC 电流采样、角度读取
3.射弹计数，过弹检测	接近传感器
4.温度调控与保护		温度传感器
5.CAN总线通信			CAN







8-11

1.定时器触发ADC，周期性检测三相电流
要点1：需要在三相下导通时进行检测
 - TIM0 的 CH012 用于驱动三相，而且使用 “中央对齐模式 + PWM模式0”（先高后低），形成“凹”的波形
 - TIM0 的 pre 暂取 2（60MHz），arr 暂取 1500，占空比取值为 200-1300，预留 200 的“下导通”时间（3.3us）
 - 为了确保在三相下导通时检测，ADC 需要使用定时器触发，触发的定时器 pre 和 arr 同上，ccr 取 1350






* adc 不用 dma 了，太麻烦

































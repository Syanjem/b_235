8.20 板子变化：
检流电路: ±10A -> ±0.5V（+1.5V）
ADC: IC	PB0
	 IB	PB1



8.21 大改
1.修改 adc 的工作配置以及与 timer 的触发方式
 - adc0 注入组，3个通道，PA2(PV)、PB0(Ic)、PB1(Ib)
 - adc0 由 timer0 的 up 或者 cp3 触发，我选择 cp3
2.修改角度采集和算法的工作



spi0 软件读写磁编
timer0 ch012 输出 pwm 驱动 adc
timer0 ch3 更新触发 adc0
adc0 注入组测量 ic ib PV
adc0 在中断服务函数里获取电流和电压、滤波、获取角度、Foc





8.25 0.5A 实际 > 1.0A
*** 硬件故障：磁编码器输出固定数据




8.31
*** 硬件修复：磁编码器恢复功能（？）
*** 功能阶段：电流环正常定性工作

*** 任务：
 1. 添加【待机检测模式】和【辅助供弹模式】




0902
2112：Angle_Feedback.c#L61-L65 
float a = pa->mangle_raw - pa->angleDf_float_get(;
修正：
float a = pa->mangle_dirfixed - pa->angleDf_float_get(;

更新事件：用户手册 
291-292
 1.重复计数器是用来在 N + 1 个计数周期之后产生更新事件，更新定时器的寄存器。
 2.重复计数器在每次计数器上溢和下溢时递减（中央对齐模式一个周期递减两次）。
 （现在的程序能用，是因为在下导通时也测到了。）
327-328
 3.重复计数器计数值减为 0 时产生更新事件，影子寄存器的更新速率也会受这些位影响(前提是影子寄存器被使能)。
290
 4.当发生更新事件时，所有影子寄存器(重复计数器，计数器自动重载寄存器，预分频寄存器)都将被更新。








0903
 0835：清空 debug，函数移到 Angle_Feedback
 debug_open23_sendEa_dirFix
 test2_P_of_motor

st-link: 红 黄
		 橙 褐

spi：v g cs sck mi mo

三相：U  V  W（A B C）
	  黄 绿 蓝

0914: foc_debug

1120：添加 发送 ic，pv 函数



开环电角度来源



问题：
1.方向：
 abc(uvw)的空间方向，
 a+b-c-的磁场方向，
 a+b-c-的电流方向，
 开环的电角度方向和闭环park的电角度方向，
 电流变换中的空间矢量方向，
 电机转动的方向（需求从背面看是顺时针）
2.参数：
 pid 参数，
 pv，
 t





clarke:
 i_alpha = ia - 0.5 * (ib + ic)		= 3 * 0.5 * ia
 i_beta = sqrt(3) * 0.5 * (ib - ic)	= sqrt(3) * 0.5 * (ia + 2 * ib)
* 2/3 (0.667):
 i_alpha = ia
 i_beta = 1/sqrt(3) * (ia + 2 * ib)

parke:
 id = cos * i_alpha + sin * i_beta
 iq = -sin * i_alpha + cos * i_beta

inparke:
 i_alpha = cos * id - sin * iq
 i_beta = sin * id + cos * iq





v_alpha, v_beta, sector:（V > 0）
1:
v_alpha = V1 + 0.5 * V2
v_beta  = s(3) * 0.5 * V2;
V1 = v_alpha - 1 / s(3) * v_beta		s3/2 * v_a - 1/2 * v_b			z
V2 = 2 / s(3) * v_beta					v_b								x
2:
v_alpha = 0.5 * (V2 - V3)
v_beta  = s(3) * (V2 + V3)
V2 = v_alpha + 0.5 / s(3) * v_beta		s3/2 * v_a + 1/2 * v_b			y
V3 = - v_alpha + 0.5 / s(3) * v_beta	-(s3/2 * v_a - 1/2 * v_b)		z
3:
v_alpha = - V4 - 0.5 * V3
v_beta  = s(3) * 0.5 * V3
V3 = 2 / s(3) * v_beta					v_b								x
V4 = - v_alpha - 1 / s(3) * v_beta		-(s3/2 * v_a + 1/2 * v_b)		y
4:
v_alpha = - V4 - 0.5 * V5
v_beta  = - (s3) * 0.5 * V5
V4 = - v_alpha + 1 / s(3) * v_beta		-(s3/2 * v_a - 1/2 * v_b)		z
V5 = - 2 / s(3) * v_beta				-v_b							x
5:
v_alpha = 0.5 * (V6 - V5)
v_beta  = - s(3) * (V6 + V5)
V5 = - v_alpha - 0.5 / s(3) * v_beta	-(s3/2 * v_a + 1/2 * v_b)		y
V6 = v_alpha - 0.5 / s(3) * v_beta		-(s3/2 * v_a - 1/2 * v_b)		z
6:
v_alpha = V1 + 0.5 * V6
v_beta  = - s(3) * 0.5 * V6
V6 = - 2 / s(3) * v_beta				-v_b							x
V1 = v_alpha + 1 / s(3) * v_beta		s3/2 * v_a + 1/2 * v_b			y











0907 修改：run, can, adc
现在所有的功能函数，都在 adc 中断服务函数里调用。
先用匀速转
1.run：上电，直接转
2.can：上电，接收到 can 信号（gpio 中断信号），开始转
3.adc：上电，adc 检测到电流变化，开始转


新增功能：
 1.spi 输出实时 speed，idq
 2.标幺化








0909 修改
1.数据形式
2.影响关系




【标幺化】
1.环的输入输出数据（速度、电流、电压需要是标幺化的数据）
2.目标值和反馈值都要进行标幺化


修改：
0909
1647：pid 限幅参数
pid_3loops:	(o, i) (1.0f, 0.9f)

2229: 数据 - 算法 - 数据
原始数据 - uint_t数据 - float数据 - 标幺化数据(float) - float数据 - uint_t数据 - 输出数据
			中间数据	物理数据		算法数据		物理数据	ATK数据


0843：记录零点偏移




2252: 去除法


















9.15
 转子会追磁场方向转动。
 电角度坐标系的零点为【A出】，方向为【A -> B -> C -> A】。
 
1.确定【方向 direction】
 制造一个电角度顺时针方向转动的磁场，【A-BC+，B-CA+，C-AB+】是一个方案。
 这里有一点，【A-BC+的电流方向为A出，但我怎么知道，此时的电角度是 0 还是 180？】
 【假设此时磁场方向为A出】，【电流与磁场同向】
 由【A-BC+，B-CA+，C-AB+】获取 0-360 电角度输出占空比
 








0917：
 确定的：【ABC三相静止坐标系的三个轴线在空间上分别固定于0°、120°、240°电角度位置。】
 
1.校准磁编的方向：输出A-B-C-A方向的磁场，观察磁编的读数变化并校准。
2.校准零点：输出A出（0°）的磁场，记录磁编的读数。
假设，【电流与磁场同向】
	A	B	C
 1	-	+	+
 2	-	-	+
 3	+	-	+
 4	+	-	-
 5	+	+	-
 6	-	+	-
电流环，iq+正转，iq-反转。
 
如果，【电流与磁场反向，但是程序当成同向】
 校准方向的部分没问题，得出的方向与实际是一样的。
 校准零点输出A出（0°）的磁场A-BC+，实际指向180°，测出的零点实际是【180°】
 
 
 09171050: 过流保护。
 
 下一步需求
 
 轴承不固定
 养蛋鸡
 
 单发射击
 
 两个计蛋数器
 
 输入分解
 
 
 










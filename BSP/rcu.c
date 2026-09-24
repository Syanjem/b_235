#include "rcu.h"

// AHB	120MHz
// APB1	60MHz
// APB2	120MHz
void rcu_init(void)
{
	// 1. 使能并等待外部晶振(HXTAL)稳定
	rcu_osci_on(RCU_HXTAL);                                   // 开启外部晶振[citation:3][citation:9]
	while(SUCCESS != rcu_osci_stab_wait(RCU_HXTAL));          // 等待晶振就绪[citation:6][citation:7]

	// 2. 配置PLL
	rcu_pll_config(RCU_PLLSRC_HXTAL_IRC48M, RCU_PLL_MUL15);          // 配置PLL源为HXTAL，倍频系数15 (例如8M*15=120M)[citation:2][citation:7]

	// 3. 使能并等待PLL锁定
	rcu_osci_on(RCU_PLL_CK);                                  // 开启PLL[citation:9]
	while(SUCCESS != rcu_osci_stab_wait(RCU_PLL_CK));                    // 等待PLL锁定[citation:7][citation:9]

	// 4. 配置AHB、APB1、APB2总线分频
	rcu_ahb_clock_config(RCU_AHB_CKSYS_DIV1);                 // AHB = 系统时钟 / 1[citation:1][citation:7][citation:9]
	rcu_apb1_clock_config(RCU_APB1_CKAHB_DIV2);               // APB1 = AHB时钟 / 2[citation:1][citation:7][citation:9]
	rcu_apb2_clock_config(RCU_APB2_CKAHB_DIV1);               // APB2 = AHB时钟 / 1[citation:1][citation:7][citation:9]

	// 5. 切换系统时钟源为PLL输出
	rcu_system_clock_source_config(RCU_CKSYSSRC_PLL);         // 系统时钟切换至PLL[citation:3][citation:7][citation:9]
	while(RCU_SCSS_PLL != rcu_system_clock_source_get()); // 等待切换完成[citation:1][citation:7]

	// 6. 更新全局系统时钟变量
	SystemCoreClockUpdate();                                  // 更新系统核心时钟变量[citation:9]
}




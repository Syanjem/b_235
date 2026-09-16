#include "Temperature_monitor.h"

/* 复用MB1601B.c的全局温度变量作为唯一数据源(已在MB1601B.h中extern) */

static uint32_t		s_last_tick = 0;		/* 上次采样时刻 */
static uint8_t		s_online = 0u;			/* 传感器在线标志 */
static Temp_Status	s_status = TEMP_OFFLINE;	/* 最近一次采样状态 */


/*** 主功能 ***/

/* 模块初始化: 引脚配置 + 连续转换模式 + 首次采样 */
void temperature_monitor_init(void)
{
	MB1601B_Init();
	MB1601B_Write_Scratchpad();		/* 配置0x0D: 每秒2次连续转换, 并写入EEPROM */

	delay_1ms(600);					/* 等待首次内部转换完成(转换周期500ms) */

	s_last_tick = get_ms_ticks();
	if (0 == MB1601B_GetTemperature(&temperature))
	{
		s_online = 1u;
		s_status = (temperature >= TEMP_OVERHEAT_THRESHOLD) ? TEMP_OVERHEAT : TEMP_OK;
	}
	else
	{
		s_online = 0u;
		s_status = TEMP_OFFLINE;
	}
}

/* 主循环周期调用: 内部按500ms节流, 到点才真正读传感器 */
Temp_Status temperature_monitor_process(void)
{
	/* 未到采样周期: 直接返回上次状态, 不阻塞 */
	if (get_ms_ticks() - s_last_tick < TEMP_MONITOR_PERIOD_MS)
	{
		return s_status;
	}
	s_last_tick = get_ms_ticks();

	/* 读暂存器: 传感器内部已自动转换, 无需发0x44 */
	if (0 != MB1601B_GetTemperature(&temperature))
	{
		s_online = 0u;
		s_status = TEMP_OFFLINE;	/* 无应答: 掉线, temperature保留上次有效值 */
		return s_status;
	}
	s_online = 1u;

	/* 过温判断 */
	s_status = (temperature >= TEMP_OVERHEAT_THRESHOLD) ? TEMP_OVERHEAT : TEMP_OK;
	return s_status;
}

/* 获取最新温度值 */
float temperature_monitor_get(void)
{
	return temperature;
}

/* 传感器在线状态 */
uint8_t temperature_monitor_is_online(void)
{
	return s_online;
}

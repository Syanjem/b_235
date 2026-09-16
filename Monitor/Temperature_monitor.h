#ifndef _TEMPERATURE_MONITOR_H_
#define _TEMPERATURE_MONITOR_H_

#include "gd32f30x_libopt.h"
#include "systick.h"
#include "temperature_1wire.h"

/* 采样周期: 与传感器内部500ms转换周期(配置0x0D)匹配, 读快了也只是重复值 */
#define TEMP_MONITOR_PERIOD_MS		500u

/* 过温阈值: 按电机/功率器件规格修改 */
#define TEMP_OVERHEAT_THRESHOLD		80.0f

typedef enum
{
	TEMP_OK = 0,		/* 温度正常 */
	TEMP_OVERHEAT,		/* 温度超过过温阈值 */
	TEMP_OFFLINE		/* 传感器无应答(掉线), 保留上次有效温度 */
} Temp_Status;

/* 模块初始化: 配置传感器引脚+连续转换模式+首次采样(内部阻塞约0.7s, 只在上电流程调用) */
void temperature_monitor_init(void);

/* 主循环周期调用: 内部按500ms节流采样, 高频调用安全, 返回最近一次采样状态 */
Temp_Status temperature_monitor_process(void);

/* 获取最新温度值(掉线时返回最后一次有效值) */
float temperature_monitor_get(void);

/* 传感器在线状态: 1=在线 0=掉线 */
uint8_t temperature_monitor_is_online(void);

#endif

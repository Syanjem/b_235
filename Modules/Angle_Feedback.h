#ifndef _ANGLE_FEEDBACK_H_
#define _ANGLE_FEEDBACK_H_

#include "timer.h"
#include "systick.h"
#include "general_def.h"

#include "angle.h"

//#include "Motor_ctrl.h"

// 角度结构体
// 角度由磁编获取
typedef struct
{
	float Speed_Base;		// rpm
	
	int8_t	 direction;		// 方向
	uint16_t p;				// 极对数
	float angle_zero;	// 零点偏移
	
	float angle_cs;		// 磁编角度
	float angle_df;		// 方向校准角度
	float angle_ma;		// 机械角度（校准）
	float angle_ma_pre;	// 上一次的机械角度
	float speed;		// 角速度
	float speed_pre;		
	float angle_ea;		// 电角度
	
	uint16_t angle_cs_debug;
	uint16_t angle_step_debug;	
	uint16_t angle_ea_debug;
} Angle_Struct;
extern Angle_Struct	angle_s;



void Angle_Feedback_Update(Angle_Struct* pa);
void Speed_Feedback_Update(Angle_Struct* pa);












#endif

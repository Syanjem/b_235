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
    float    speed_base;        // 基速，rpm
    int8_t   direction;         // 方向
    uint16_t pole_pairs;        // 极对数
    float    angle_zero;        // 零点偏移

    float    angle_encoder;     // 磁编角度
    float    angle_dir_calib;   // 方向校准角度
    float    angle_mech;        // 机械角度（校准）
    float    angle_mech_prev;   // 上一次的机械角度
    float    speed;             // 角速度
    float    speed_prev;        // 上一次的角速度
    float    angle_elec;        // 电角度

    uint16_t angle_encoder_debug;
    uint16_t angle_step_debug;
    uint16_t angle_elec_debug;
} angle_state_t;

extern angle_state_t g_angle;



void Angle_Feedback_Update(angle_state_t* pa);
void Speed_Feedback_Update(angle_state_t* pa);












#endif

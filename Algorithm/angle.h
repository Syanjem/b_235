#ifndef _ANGLE_H_
#define _ANGLE_H_

#include "systick.h"
#include "encoder_spi.h"
#include "ATK.h"

void angleDf_float_fix(float* a_df, int8_t direction, float a_cs);
void angleZero_float_get(float* a_zero, int8_t direction);







/***   调试功能   ***/
/***   调试功能   ***/
/***   调试功能   ***/

// 开环转动、测试磁编、测试方向
void debug_open23_sendEa_dirFix(uint16_t st);
// 测试极对数
void debug_P_of_motor(void);
void motor_to_eangle(uint16_t ea);
void set_pwm_eangle_arrabc(uint16_t);
extern uint16_t en1500_arrabc_array[361][4];

#endif

#ifndef _SVPWM_H_
#define _SVPWM_H_

#include "stdint.h"
#include "math.h"

#include "general_def.h"

/* 电压状态 */
typedef struct
{
    float    u_base;          // 电压基值

    float    v_d;             // 电流环输出 d 轴电压
    float    v_q;             // 电流环输出 q 轴电压
    float    modulation;      // 调制比（速度环 |vq| / MAX_V）
    float    v_alpha;         // Park 逆变换 alpha 轴
    float    v_beta;          // Park 逆变换 beta 轴
    uint8_t  sector;          // 扇区（由 v_alpha, v_beta 判断）
} voltage_state_t;

extern voltage_state_t g_voltage;

/* 三相 PWM 占空比 */
typedef struct
{
    uint16_t period;          // PWM 周期
    uint16_t duty_min;        // 占空比下限
    uint16_t duty_max;        // 占空比上限
    uint16_t duty_span;       // duty_max - duty_min
    uint16_t duty_a;          // A 相占空比
    uint16_t duty_b;          // B 相占空比
    uint16_t duty_c;          // C 相占空比
} pwm_duty_t;

extern pwm_duty_t g_pwm_duty;



void v_update(voltage_state_t* pv, float t_vq, float t_vd, float vbus_V, float ea);



















void pwm_output_update(voltage_state_t* pv, pwm_duty_t* pabc);
void svpwm_sector(float alpha, float beta, uint8_t* ps);
void svpwm_v123t12(float v_alpha, float v_beta, int8_t sector, float* pT1, float* pT2);
void svpwm_abcduty(float T1, float T2, int8_t s, float k, uint16_t period, uint16_t *pTA, uint16_t *pTB, uint16_t *pTC);
void set_pwm_abc(pwm_duty_t* pabc);








void pwm_output_update_debug(voltage_state_t* pv, pwm_duty_t* pabc);




#endif

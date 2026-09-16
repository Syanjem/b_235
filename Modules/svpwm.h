#ifndef _SVPWM_H_
#define _SVPWM_H_

#include "stdint.h"
#include "math.h"

#include "general_def.h"

// 电压结构体 
// vd,vq 由 adc 获取
typedef struct
{
    float v_d;				// 电流环输出
    float v_q;				// 电流环输出
	float k;				// （暂时由速度环 |vq| / MAX_V 得到）
	float v_alpha;			// V输出：Park 逆变换
	float v_beta;			// V输出：Park 逆变换
	uint8_t sector;			// V输出：v_alpha,v_beta 判断
} V_Struct; 	
extern V_Struct	v_s;

// 占空比结构体
// 参数均在初始化中确定（周期，占空比区间）
// 在输出vdq结构体后，计算更新abc结构体
typedef struct
{
	uint16_t period;	// 周期
	uint16_t begin;		// 占空比下限
	uint16_t end;		// 占空比上限
	uint16_t T;			// end - start
    uint16_t duty_a;	// V,ABC输出：由sector,vab,k计算得到
    uint16_t duty_b;	// V,ABC输出：由sector,vab,k计算得到
    uint16_t duty_c;	// V,ABC输出：由sector,vab,k计算得到
} ABCpwm_Struct;
extern ABCpwm_Struct	abc_s;
void ABCpwm_Struct_init(uint16_t p, uint16_t b, uint16_t e,
						uint16_t da, uint16_t db, uint16_t dc,
						ABCpwm_Struct* pd);

void pwm_output_update(V_Struct* pv, ABCpwm_Struct* pabc);
void SVPWM_Sector(float alpha, float beta, uint8_t* ps);
void SVPWM_V123T12(float v_alpha, float v_beta, int8_t sector, float* pT1, float* pT2);
void SVPWM_ABCDuty(float T1, float T2, int8_t s, float k, uint16_t period, uint16_t *pTA, uint16_t *pTB, uint16_t *pTC);
void set_pwm_abc(ABCpwm_Struct* pabc);








void pwm_output_update_debug(V_Struct* pv, ABCpwm_Struct* pabc);




#endif

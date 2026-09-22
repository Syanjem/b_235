#ifndef _IDQ_FEEDBACK_H_
#define _IDQ_FEEDBACK_H_

#include "gd32f30x_libopt.h"
#include "adc.h"

#include "general_def.h"

typedef struct
{
	float I_Base;
	
	uint16_t ic_shot;
	uint16_t ib_shot;
	uint16_t i_offset;
	uint16_t pv_shot;
	
	float ia;
	float ib;
	float i_alpha;
	float i_beta;
	float vbus_V; 
	
	float id;
	float iq;
} Idq_Struct;
extern Idq_Struct idq_s;


void i_shot_form_adc0inserted(Idq_Struct* pi);



#define MEDIAN_N  3   // 中值滤波：取最近5个值排序
#define AVG_N     1  // 滑动平均：取最近10个中值滤波后的结果求平均
typedef struct
{
	uint8_t  med_index_p;         // 中值滤波器写指针
	uint8_t  avg_index_p;         // 滑动平均滤波器写指针

	uint16_t adc_buf[MEDIAN_N];     // 原始ADC值，用于中值滤波器
	uint16_t med_out_buf[AVG_N];    // 中值滤波输出值，用于滑动平均滤波器
	uint32_t sum;               // 滑动平均滤波累加和（用于求平均值，每次更新）
} Filter_Struct;
extern Filter_Struct filter_s_ic;
extern Filter_Struct filter_s_ib; 
extern Filter_Struct filter_s_pv;
uint16_t get_filtered(uint16_t raw_adc, Filter_Struct* pfs); // icb_dma_buffer2[0]







void Idq_Feedback_Update(Idq_Struct* pi, float ea);


void bubble_sort(uint16_t arr[], uint8_t len);



#endif

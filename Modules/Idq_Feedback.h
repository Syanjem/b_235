#ifndef _IDQ_FEEDBACK_H_
#define _IDQ_FEEDBACK_H_

#include "gd32f30x_libopt.h"
#include "adc.h"

#include "general_def.h"

typedef struct
{
    float    i_base;          // 电流基值

    uint16_t i_c_sample;      // C 相电流采样
    uint16_t i_b_sample;      // B 相电流采样
    uint16_t i_offset;        // 电流偏置
    uint16_t vbus_sample;     // 母线电压采样

    float    i_a;             // A 相电流
    float    i_b;             // B 相电流
    float    i_alpha;         // Clark 变换 alpha 轴
    float    i_beta;          // Clark 变换 beta 轴
    float    vbus;            // 母线电压

    float    i_d;             // Park 变换 d 轴
    float    i_q;             // Park 变换 q 轴
} current_state_t;

extern current_state_t g_current;



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







void idq_feedback_update(current_state_t* pi, float ea);


void bubble_sort(uint16_t arr[], uint8_t len);



#endif

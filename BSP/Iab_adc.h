#ifndef _IAB_ADC_H_
#define _IAB_ADC_H_

#include "gd32f30x_libopt.h"
#include "adc.h"
#include "spi.h"




typedef struct
{
	float ia;
	float ib;
	float cos_angle_J;
	float sin_angle_J;
	float id;
	float iq;
} I_Typedef;
extern I_Typedef i_typedef;
void Iab_get(I_Typedef* pI, float i_offset);
void IBuffer_Init(void);

#define MEDIAN_N  5   // 中值滤波：取最近5个值排序
#define AVG_N     10  // 滑动平均：取最近10个中值滤波后的结果求平均


extern uint16_t adc_buf_ic[MEDIAN_N];     // 存储原始ADC值（用于中值滤波）
extern uint8_t  med_index_ic;         // 中值滤波的写入指针

extern uint16_t med_out_buf_ic[AVG_N];    // 存储中值滤波后的值（用于滑动平均）
extern uint8_t  avg_index_ic;         // 滑动平均的写入指针
extern uint32_t sum_ic;               // 滑动平均的累加和（用递推法避免每次重算）

extern uint16_t adc_buf_ia[MEDIAN_N];     // 存储原始ADC值（用于中值滤波）
extern uint8_t  med_index_ia;         // 中值滤波的写入指针

extern uint16_t med_out_buf_ia[AVG_N];    // 存储中值滤波后的值（用于滑动平均）
extern uint8_t  avg_index_ia;         // 滑动平均的写入指针
extern uint32_t sum_ia;               // 滑动平均的累加和（用递推法避免每次重算）


uint16_t get_filtered_ic(uint16_t raw_adc); // ica_dma_buffer[0]
uint16_t get_filtered_ia(uint16_t raw_adc); // ica_dma_buffer[1]
void bubble_sort(uint16_t arr[], uint8_t len);


#endif

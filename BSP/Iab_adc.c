#include "Iab_adc.h"

I_Typedef i_typedef;

void Iab_get(I_Typedef* pI, float i_offset)
{
	if (ica_dma_out == 1)
	{
		__disable_irq();
		
        uint16_t ic_f = get_filtered_ic((uint16_t)( ica_dma_buffer2        & 0xFFFFu));  // 低16位 = ADC0 = Ic
        uint16_t ia_f = get_filtered_ia((uint16_t)((ica_dma_buffer2 >> 16) & 0xFFFFu));  // 高16位 = ADC1 = Ia
		
		SPI_NSS2_SELECT();
		SPI_ReadWriteWord(SPI0, (uint16_t)( ica_dma_buffer2        & 0xFFFFu));
		SPI_ReadWriteWord(SPI0, (uint16_t)((ica_dma_buffer2 >> 16) & 0xFFFFu));
//		SPI_ReadWriteWord(SPI0, ic_f);
//		SPI_ReadWriteWord(SPI0, ia_f);
		SPI_NSS2_DESELECT();
		
		pI->ia = ((float)ia_f / 4095.0f) * 3.3f - i_offset;
		pI->ib = -((float)ic_f / 4095.0f) * 3.3f + i_offset - pI->ia;

		ica_dma_out = 0;
		__enable_irq();
	}
}


//void IBuffer_Init(void)
//{
//	for (uint8_t i = 10; i > 0; i--)
//	{
//		get_filtered_ic(ica_dma_buffer2[0]);
//		get_filtered_ia(ica_dma_buffer2[1]);
//	}
//}




uint16_t adc_buf_ic[MEDIAN_N];     // 储存原始ADC值，用于中值滤波器
uint8_t  med_index_ic = 0;         // 中值滤波器写指针

uint16_t med_out_buf_ic[AVG_N];    // 储存中值滤波输出值，用于滑动平均滤波器
uint8_t  avg_index_ic = 0;         // 滑动平均滤波器写指针
uint32_t sum_ic = 0;               // 滑动平均滤波累加和（用于求平均值，每次更新）

uint16_t get_filtered_ic(uint16_t raw_adc) // ica_dma_buffer2[0]
{	
    uint16_t temp_arr[MEDIAN_N];
    uint16_t median_value;

    // ================= 第一级（中值滤波，去尖峰） =================
    // 1. 将ADC值放入中值滤波器
    adc_buf_ic[med_index_ic] = raw_adc;
    med_index_ic = (med_index_ic + 1) % MEDIAN_N;
    
    // 2. 复制当前缓冲区数据到临时数组并排序
    for (uint8_t i = 0; i < MEDIAN_N; i++) temp_arr[i] = adc_buf_ic[i];
    bubble_sort(temp_arr, MEDIAN_N);
    
    // 3. 取中间值（中值滤波器输出）
    median_value = temp_arr[MEDIAN_N / 2];  // N=5时，取下标2

    // ================= 第二级（滑动平均滤波，平滑波动） =================
    // 1. 将中值滤波结果加入滑动平均滤波器（循环缓冲）
    sum_ic = sum_ic - med_out_buf_ic[avg_index_ic];      // 减去最旧样本的值
    med_out_buf_ic[avg_index_ic] = median_value;   // 写入新值
    sum_ic = sum_ic + median_value;                // 加上新值
    avg_index_ic = (avg_index_ic + 1) % AVG_N;

    // 2. 返回滑动平均值（即实际使用的电流值）
    return (uint16_t)(sum_ic / AVG_N);
}



uint16_t adc_buf_ia[MEDIAN_N];     // 储存原始ADC值，用于中值滤波器
uint8_t  med_index_ia = 0;         // 中值滤波器写指针

uint16_t med_out_buf_ia[AVG_N];    // 储存中值滤波输出值，用于滑动平均滤波器
uint8_t  avg_index_ia = 0;         // 滑动平均滤波器写指针
uint32_t sum_ia = 0;               // 滑动平均滤波累加和（用于求平均值，每次更新）

uint16_t get_filtered_ia(uint16_t raw_adc)	// ica_dma_buffer[1]
{	
    uint16_t temp_arr[MEDIAN_N];
    uint16_t median_value;

    // ================= 第一级（中值滤波，去尖峰） =================
    // 1. 将ADC值放入中值滤波器
    adc_buf_ia[med_index_ia] = raw_adc;
    med_index_ia = (med_index_ia + 1) % MEDIAN_N;
    
    // 2. 复制当前缓冲区数据到临时数组并排序
    for (uint8_t i = 0; i < MEDIAN_N; i++) temp_arr[i] = adc_buf_ia[i];
    bubble_sort(temp_arr, MEDIAN_N);
    
    // 3. 取中间值（中值滤波器输出）
    median_value = temp_arr[MEDIAN_N / 2];  // N=5时，取下标2

    // ================= 第二级（滑动平均滤波，平滑波动） =================
    // 1. 将中值滤波结果加入滑动平均滤波器（循环缓冲）
    sum_ia = sum_ia - med_out_buf_ia[avg_index_ia];      // 减去最旧样本的值
    med_out_buf_ia[avg_index_ia] = median_value;   // 写入新值
    sum_ia = sum_ia + median_value;                // 加上新值
    avg_index_ia = (avg_index_ia + 1) % AVG_N;

    // 2. 返回滑动平均值（即实际使用的电流值）
    return (uint16_t)(sum_ia / AVG_N);
}



// 上面用到的中值滤波冒泡排序（N=5时很快）
void bubble_sort(uint16_t arr[], uint8_t len) 
{
    for (uint8_t i = 0; i < len - 1; i++) {
        for (uint8_t j = 0; j < len - 1 - i; j++) {
            if (arr[j] > arr[j + 1]) {
                uint16_t temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}








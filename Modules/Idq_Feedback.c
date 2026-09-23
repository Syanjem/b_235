#include "Idq_Feedback.h"

// get: [id, iq]
current_state_t g_current = {
	.i_base = 33.0f,	// 标幺值范围是 [-1, +1]
	
	.i_c_sample	= 0u,
	.i_b_sample	= 0u,
	.i_offset	= 2048u,
	.vbus_sample	= 0u,
	
	.i_a			= 0.0f,
	.i_b			= 0.0f,
	.i_alpha	= 0.0f,
	.i_beta		= 0.0f,
	.vbus		= 1.0f,
	
	.i_d			= 0.0f,
	.i_q			= 0.0f,
};




// get [ic_raw, ib_raw]

void i_shot_form_adc0inserted(current_state_t* pi)
{
	pi->i_c_sample = adc_inserted_data_read(ADC0, ADC_INSERTED_CHANNEL_0);	// 对应 JDR1/IDATA0
	pi->i_b_sample = adc_inserted_data_read(ADC0, ADC_INSERTED_CHANNEL_1);	// 对应 JDR2/IDATA1
	pi->vbus_sample = adc_inserted_data_read(ADC0, ADC_INSERTED_CHANNEL_2);	// 对应 JDR3/IDATA2

	static 	uint8_t adc_init_num = 0;
	if (adc_init_num < MEDIAN_N)	// 前 MEDIAN_N 个数据采样原始值
	{
		filter_s_ic.adc_buf[adc_init_num] = pi->i_c_sample;
		filter_s_ib.adc_buf[adc_init_num] = pi->i_b_sample;
		filter_s_pv.adc_buf[adc_init_num] = pi->vbus_sample;
		adc_init_num++;
	}
	else // 后面的数据采样滤波值
	{
		pi->i_c_sample = get_filtered(pi->i_c_sample, &filter_s_ic);
		pi->i_b_sample = get_filtered(pi->i_b_sample, &filter_s_ib);
		pi->vbus_sample = get_filtered(pi->vbus_sample, &filter_s_pv);
	}
}




// [ic_raw, ib_raw, ea] to [iq, id]
void Idq_Feedback_Update(current_state_t* pi, float ea)
{
	i_shot_form_adc0inserted(pi);
	
	// 相电流换算: ADC raw -> 电压 -> 减零点偏置(offset) -> 电流(单位 A)
	// (ic, ib) 获取 (ia, ib)
	pi->i_b = ((float)(pi->i_b_sample - 2048) / 2048.0f);	// pu [-1, +1]
	float c = ((float)(pi->i_c_sample - 2048) / 2048.0f);
	pi->i_a = - (pi->i_b + c);
	
	pi->vbus = ((float)pi->vbus_sample / 4095.0f) * 3.3f * 11.0f;
	
	// clarke 变换，获取 (i_alpha, i_beta) 
	clarke_transform(pi->i_a, pi->i_b, &pi->i_alpha, &pi->i_beta);
	// park 变化，获取 (id, iq)
	park_transform(pi->i_alpha, pi->i_beta, ea * M_PI / 180.0f, &pi->i_d, &pi->i_q);
}



Filter_Struct filter_s_ic = {
	.med_index_p = 0,        					 // 中值滤波器写指针
	.avg_index_p = 0,        					 // 滑动平均滤波器写指针

	.adc_buf = {2048, 2048, 2048},     // 原始ADC值，用于中值滤波器
	.med_out_buf = {2048},    // 中值滤波输出值，用于滑动平均滤波器
	.sum = 2048u * AVG_N,             						  // 滑动平均滤波累加和（用于求平均值，每次更新）
};

Filter_Struct filter_s_ib = {
	.med_index_p = 0,        					 // 中值滤波器写指针
	.avg_index_p = 0,        					 // 滑动平均滤波器写指针

	.adc_buf = {2048, 2048, 2048},     // 原始ADC值，用于中值滤波器
	.med_out_buf = {2048},    // 中值滤波输出值，用于滑动平均滤波器
	.sum = 2048u * AVG_N,             						  // 滑动平均滤波累加和（用于求平均值，每次更新）
};

Filter_Struct filter_s_pv = {
	.med_index_p = 0,        					 // 中值滤波器写指针
	.avg_index_p = 0,        					 // 滑动平均滤波器写指针

	// pv 是单极性信号, 母线 24V 经 1/11 分压 -> 2.18V -> ADC raw ≈ 2707
	// 初始值填正常工作点读数, 开机首拍滤波输出即为真实值, 无收敛过程
	.adc_buf = {2707, 2707, 2707},
	.med_out_buf = {2707},
	.sum = 2707u * AVG_N,
};


uint16_t get_filtered(uint16_t raw_adc, Filter_Struct* pfs) // icb_dma_buffer2[0]
{	
    uint16_t temp_arr[MEDIAN_N];
    uint16_t median_value;
	
    // ================= 第一级（中值滤波，去尖峰） =================
    // 1. 将ADC值放入中值滤波器
    pfs->adc_buf[pfs->med_index_p] = raw_adc;
    pfs->med_index_p = (pfs->med_index_p + 1) % MEDIAN_N;
    
    // 2. 复制当前缓冲区数据到临时数组并排序
    for (uint8_t i = 0; i < MEDIAN_N; i++) temp_arr[i] = pfs->adc_buf[i];
    bubble_sort(temp_arr, MEDIAN_N);
    
    // 3. 取中间值（中值滤波器输出）
    median_value = temp_arr[MEDIAN_N / 2];  // N=5时，取下标2

    // ================= 第二级（滑动平均滤波，平滑波动） =================
    // 1. 将中值滤波结果加入滑动平均滤波器（循环缓冲）
    pfs->sum = pfs->sum - pfs->med_out_buf[pfs->avg_index_p];      // 减去最旧样本的值
    pfs->med_out_buf[pfs->avg_index_p] = median_value;   // 写入新值
    pfs->sum = pfs->sum + median_value;                // 加上新值
    pfs->avg_index_p = (pfs->avg_index_p + 1) % AVG_N;

    // 2. 返回滑动平均值（即实际使用的电流值）
    return (uint16_t)(pfs->sum / AVG_N);
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



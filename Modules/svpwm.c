#include "svpwm.h"

voltage_state_t g_voltage = {
	.u_base = 24.0f,	// = vbus/2, 相电压峰值基准, SVPWM 线性区匹配
	
	.v_d 		= 0.0f,
	.v_q		= 1.0f,
	.modulation = 0.5,
	.v_alpha	= 0.0f,
	.v_beta		= 1.0f,	
	.sector		= 2u,
};

pwm_duty_t g_pwm_duty = {
	.period		= 3000,
	.duty_min	= 1200,
	.duty_max	= 2700,
	.duty_span	= 1000,
	.duty_a		= 1500,
	.duty_b		= 2000,
	.duty_c		= 2500,
};


// pv 从 pa 获取电角度，计算三角函数
void v_update(voltage_state_t* pv, float t_vq, float t_vd, float vbus_V, float ea)
{
	pv->v_q = t_vq;	// pu
	pv->v_d = t_vd;	// pu

	// 调制比 k = sqrt(3) * |Vdq|_pu / Vbus_pu
	// vdq 是 pu, vbus 也要转 pu, 否则 k 会严重偏小
	float vdq_mag = sqrtf(pv->v_d * pv->v_d + pv->v_q * pv->v_q);
	float vbus_pu = vbus_V / pv->u_base;	// 实际 V / 基准 V = pu
	if (vbus_pu < 0.1f) vbus_pu = 0.1f;			// 兜底: 防除零
	pv->modulation = _SQRT3 * vdq_mag / vbus_pu;
	if (pv->modulation > 0.4f) pv->modulation = 0.4f;		// 过调制限幅

	inverse_park(pv->v_d, pv->v_q, ea * M_PI / 180.0f, &pv->v_alpha, &pv->v_beta);
}





void pwm_output_update(voltage_state_t* pv, pwm_duty_t* pabc)
{
	uint8_t sector;
	svpwm_sector(pv->v_alpha, pv->v_beta, &sector);
	
	float k1, k2;
	svpwm_v123t12(pv->v_alpha, pv->v_beta, sector, &k1, &k2);
	
	svpwm_abcduty(k1, k2, sector, pv->modulation, pabc->period,
					&pabc->duty_a, &pabc->duty_b, &pabc->duty_c);
	
	set_pwm_abc(pabc);
}

void svpwm_sector(float alpha, float beta, uint8_t* ps)
{
    if (alpha == 0.0f && beta == 0.0f)
    {
        *ps = 0u; // 零矢量
    }
	else
	{
		if (beta > 0.0f)
		{
			if ( ABS(beta) > ABS(_SQRT3 * alpha) )		*ps = 2u;
			else
			{
				if (alpha > 0.0f)	*ps = 1u;
				else				*ps = 3u;
			}
		}
		else 
		{
			if ( ABS(beta) > ABS(_SQRT3 * alpha) )		*ps = 5u;
			else
			{
				if (alpha > 0.0f)	*ps = 6u;
				else				*ps = 4u;
			}
		}
	}
}


void svpwm_v123t12(float v_alpha, float v_beta, int8_t sector, float* pT1, float* pT2)
{
    /* 3.计算基本矢量时间比例系数 */
    // 中间变量
    float v_1 = fabs(v_beta);
    float v_2 = fabs(_SQRT3_2 * v_alpha + 0.5f * v_beta);
    float v_3 = fabs(_SQRT3_2 * v_alpha - 0.5f * v_beta);

    // 2. 根据扇区计算Ta, Tb, Tc (作用时间)
    switch (sector)
    {
    case 1: // 0~60度: U1(011), U2(001)
        *pT1 = v_3;
        *pT2 = v_1;
        break;
    case 2: // 60~120度: U2(001), U3(101)
        *pT1 = v_2;
        *pT2 = v_3;
        break;
    case 3: // 120~180度: U3(101), U4(100)
        *pT1 = v_1;
        *pT2 = v_2;
        break;
    case 4: // 180~240度: U4(100), U5(110)
        *pT1 = v_3;
        *pT2 = v_1;
        break;
    case 5: // 240~300度: U5(110), U6(010)
        *pT1 = v_2;
        *pT2 = v_3;
        break;
    case 6: // 300~360度: U6(010), U1(011)
        *pT1 = v_1;
        *pT2 = v_2;
        break;
    default:
        *pT1 = 0;
        *pT2 = 0;
        break;
    }
}


void svpwm_abcduty(float T1, float T2, int8_t s, float k, uint16_t period, 
					uint16_t *pTA, uint16_t *pTB, uint16_t *pTC)
{
    if (T1 + T2 == 0.0f)
    {
        // 零电压矢量, 输出 50% 占空比
        *pTA = period / 2;
        *pTB = period / 2;
        *pTC = period / 2;
        return;
    }
    else
    {
        // 1. 归一化: T1/T2 是相对值, 转成实际占空比时间
        float sum = T1 + T2;
        float t1_norm = T1 / sum;  // T1 占比
        float t2_norm = T2 / sum;  // T2 占比

        // 2. k 作用于总有效矢量时间: t1+t2 = k * period
        uint16_t t_total = (uint16_t)(k * period);  // 总有效时间
        uint16_t t1 = (uint16_t)(t_total * t1_norm);
        uint16_t t2 = t_total - t1;  // 避免累计误差
        
        // 3. 零矢量时间 = period - t1 - t2, 前后各一半
        uint16_t t0 = (period - t1 - t2) / 2;

        switch (s)
        {
        case 1: // 0~60度: U1(011), U2(001)		cba（21:21）
            *pTA = t0 + t2 + t1;
            *pTB = t0 + t2;
            *pTC = t0;
            break;
        case 2: // 60~120度: U2(001), U3(101)	cab（23:12）
            *pTA = t0 + t1;
            *pTB = t0 + t1 + t2;
            *pTC = t0;
            break;
        case 3: // 120~180度: U3(101), U4(100)	acb（43:21）
            *pTA = t0;
            *pTB = t0 + t2 + t1;
            *pTC = t0 + t2;
            break;
        case 4: // 180~240度: U4(100), U5(110)	abc（45:12）
            *pTA = t0;
            *pTB = t0 + t1;
            *pTC = t0 + t1 + t2;
            break;
        case 5: // 240~300度: U5(110), U6(010)	bac（65:21）
            *pTA = t0 + t2;
            *pTB = t0;
            *pTC = t0 + t2 + t1;
            break;
        case 6: // 300~360度: U6(010), U1(011)	bca（61:12）
            *pTA = t0 + t1 + t2;
            *pTB = t0;
            *pTC = t0 + t1;
            break;
        default:
            *pTA = *pTB = *pTC = 0;
            break;
        }
    }
}


void set_pwm_abc(pwm_duty_t* pabc)
{
	
	if (TIMER0_PWM_MODE == TIMER_OC_MODE_PWM1)		// PWM1（低 高）
	{
		timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_2, pabc->duty_a);	
		timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_1, pabc->duty_b);	
		timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_0, pabc->duty_c);	
	}
	else if (TIMER0_PWM_MODE == TIMER_OC_MODE_PWM0)	// PWM0（高 低）
	{
		timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_2, TIMER0_PERIOD - pabc->duty_a);	
		timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_1, TIMER0_PERIOD - pabc->duty_b);	
		timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_0, TIMER0_PERIOD - pabc->duty_c);	
	}
}





void pwm_output_update_debug(voltage_state_t* pv, pwm_duty_t* pabc)
{
    float Va = CLAMP(pv->v_alpha, -1.0f, 1.0f);
    float Vb = CLAMP(-0.5f * pv->v_alpha + _SQRT3_2 * pv->v_beta, -1.0f, 1.0f);
    float Vc = CLAMP(-0.5f * pv->v_alpha - _SQRT3_2 * pv->v_beta, -1.0f, 1.0f);
	
	pabc->duty_a = (uint16_t)((Va + 1.0f) * 0.5f * pabc->period);	
	pabc->duty_b = (uint16_t)((Vb + 1.0f) * 0.5f * pabc->period);		
	pabc->duty_c = (uint16_t)((Vc + 1.0f) * 0.5f * pabc->period);	
	
	set_pwm_abc(pabc);
}






#include "JiaoZhun.h"

void jiaozhun_mangle_rawion(int8_t* direction)
{
	delay_1ms(10);
	
	float ma_pre;
	float ma_now;
	uint8_t up = 0u;
	uint8_t down = 0u;
	
	motor_to_eangle(0);
	delay_1ms(1);
	angle_cs_float_from_encoder(&ma_now);
	for (int i = 1; i < 100; i++)	// 99 步
	{
		ma_pre = ma_now;								// 保存 ma_pre
		motor_to_eangle(3 * i);
		delay_1ms(1);
		angle_cs_float_from_encoder(&ma_now);	// 更新 ma_now
		
		if (ma_now >= ma_pre)
		{
			up++;
		}
		else // (ma_now < ma_pre)
		{
			down++;
		}
	}
	
	if (up > down)
	{
		*direction = 1;
	}
	else //  (up < down)
	{
		*direction = -1;
	}

	delay_1ms(10);
}

//void jiaozhun_motor_P(uint8_t* pp)	// after direction
//{
//	delay_1ms(10);
//	
//	float ma_pre;
//	float ma_now;
//	float mad_buf[47];
//	float sum = 0.0f;
//	
//	motor_to_eangle(0);
//	delay_1ms(1);
//	angle_cs_float_from_encoder(&ma_now);
//	angleDf_float_fix(&ma_now, angle_s.direction);
//	for (int i = 1; i < 48; i++)	// 8圈47步（每步60度）
//	{
//		ma_pre = ma_now;
//		motor_to_eangle(60 * i % 360);
//		delay_1ms(1);
//		angle_cs_float_from_encoder(&ma_now);
//		angleDf_float_fix(&ma_now, angle_s.direction, );
//		
//		if (ma_now > ma_pre)
//		{
//			mad_buf[i - 1] = ma_now - ma_pre;
//		}
//		else // (ma_now < ma_pre)
//		{
//			mad_buf[i - 1] = 360 + ma_now - ma_pre;
//		}
//		sum += mad_buf[i - 1];
//	}
//	float d = (float)sum / 47.0f;
//	
//	*pp = (uint8_t)roundf(60.f / d);
//	
//	delay_1ms(10);
//}

// after direction, P
//void jiaozhun_mangleZero(float* pma0)	
//{
//	delay_1ms(10);
//	
//	motor_to_eangle(0);
//	angle_cs_float_from_encoder(pma0);
//	angleDf_float_fix(pma0, angle_s.direction);
//	
//	delay_1ms(10);
//}
















#include "get_idq_adc.h"

I_Typedef i_typedef = {0};

void i_typedef_init(I_Typedef* pi)
{
	pi->ia = 1;
	pi->ib = -1;
	pi->ic = 0;
	pi->cos_angle_J = 1;
	pi->sin_angle_J = 0;
	pi->id = 0;
	pi->iq = 1;
}

void get_iac_adc(I_Typedef* pI)
{
	uint16_t ia_temp = adc_inserted_data_read(ADC0, 0);
	uint16_t ic_temp = adc_inserted_data_read(ADC0, 1);
	
	pI->ia = ia_temp;
	pI->ic = ic_temp;	
	// 需要先调用set_fix_mangle(&mangle_struct);
	pI->cos_angle_J = cosf(angle_typedef.mangle_fixed * PI / 180.0f);
	pI->sin_angle_J = sinf(angle_typedef.mangle_fixed * PI / 180.0f);
	
}


void get_iac_set_idq(I_Typedef* pI)
{
	get_iac_adc(pI);
	pI->ib = - pI->ia - pI->ic;
	
	float	i_alpha = (float)pI->ia;
	float	i_beta	= (float)(SQRT_1_3 * (pI->ia + 2 * pI->ib));
	
	pI->id = pI->cos_angle_J * i_alpha + pI->sin_angle_J * i_beta;
	pI->iq = - pI->sin_angle_J * i_alpha + pI->cos_angle_J * i_beta;
}






#include "timer.h"


// PA8	CH0	 C
// PA9	CH1	 B
// PA10	CH2	 A
// PB13	CH0N
// PB14	CH1N
// PB15	CH2N

// 中央对齐向上，PWM1（低高低），update触发adc
void timer0_init(void)
{
    rcu_periph_clock_enable(RCU_TIMER0);
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_AF);
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10);
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
    timer_deinit(TIMER0);

    timer_parameter_struct		timer_initTypeDef;
    timer_struct_para_init(&timer_initTypeDef);
    timer_initTypeDef.prescaler				= 1 - 1;	
    timer_initTypeDef.period				= 3000 - 1;			
    timer_initTypeDef.alignedmode			= TIMER_COUNTER_CENTER_UP;	
    timer_initTypeDef.clockdivision			= TIMER_CKDIV_DIV1;
    timer_initTypeDef.repetitioncounter		= 1;
    timer_init(TIMER0, &timer_initTypeDef);
    timer_update_event_enable(TIMER0);
    timer_auto_reload_shadow_enable(TIMER0);

    // 死区设置
    timer_break_parameter_struct		timer_bk_initTypeDef;
    timer_break_struct_para_init(&timer_bk_initTypeDef);
    timer_bk_initTypeDef.deadtime 			= TIMER0_DEADTIME;
    timer_bk_initTypeDef.runoffstate 		= TIMER_ROS_STATE_DISABLE;
    timer_bk_initTypeDef.ideloffstate 		= TIMER_IOS_STATE_DISABLE;
    timer_bk_initTypeDef.breakpolarity 		= TIMER_BREAK_POLARITY_LOW;
    timer_bk_initTypeDef.outputautostate	= TIMER_OUTAUTO_ENABLE;
    timer_bk_initTypeDef.protectmode 		= TIMER_CCHP_PROT_0;
    timer_bk_initTypeDef.breakstate 		= TIMER_BREAK_DISABLE;
    timer_break_config(TIMER0, &timer_bk_initTypeDef);
    timer_break_enable(TIMER0);

    // 输出比较模式
    timer_oc_parameter_struct		timer_oc_initTypeDef;
    timer_channel_output_struct_para_init(&timer_oc_initTypeDef);
    timer_oc_initTypeDef.outputstate	= TIMER_CCX_ENABLE;
    timer_oc_initTypeDef.outputnstate	= TIMER_CCXN_ENABLE;
    timer_oc_initTypeDef.ocpolarity		= TIMER_OC_POLARITY_HIGH;
    timer_oc_initTypeDef.ocnpolarity 	= TIMER_OCN_POLARITY_HIGH;
    timer_oc_initTypeDef.ocidlestate 	= TIMER_OC_IDLE_STATE_LOW;
    timer_oc_initTypeDef.ocnidlestate 	= TIMER_OCN_IDLE_STATE_LOW;

    // CH0 C
    timer_channel_output_config(TIMER0, TIMER_CH_0, &timer_oc_initTypeDef);
    timer_channel_output_mode_config(TIMER0, TIMER_CH_0, TIMER0_PWM_MODE);					// PWM1
    timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_0, TIMER0_CH0_CCR);	
    timer_channel_output_shadow_config(TIMER0, TIMER_CH_0, TIMER_OC_SHADOW_ENABLE);

    // CH1 B
    timer_channel_output_config(TIMER0, TIMER_CH_1, &timer_oc_initTypeDef);
    timer_channel_output_mode_config(TIMER0, TIMER_CH_1, TIMER0_PWM_MODE);					// PWM1
    timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_1, TIMER0_CH1_CCR);
    timer_channel_output_shadow_config(TIMER0, TIMER_CH_1, TIMER_OC_SHADOW_ENABLE);

    // CH2 A
    timer_channel_output_config(TIMER0, TIMER_CH_2, &timer_oc_initTypeDef);
    timer_channel_output_mode_config(TIMER0, TIMER_CH_2, TIMER0_PWM_MODE);					// PWM1
    timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_2, TIMER0_CH2_CCR);
    timer_channel_output_shadow_config(TIMER0, TIMER_CH_2, TIMER_OC_SHADOW_ENABLE);	

    timer_master_output_trigger_source_select(TIMER0, TIMER_TRI_OUT_SRC_UPDATE);

    timer_primary_output_config(TIMER0, ENABLE);
    timer_disable(TIMER0);
}











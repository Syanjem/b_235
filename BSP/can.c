#include "can.h"

// APB1: 60MHz（rcu.c）
void CAN_Init(void)
{
	rcu_periph_clock_enable(RCU_CAN0);
	rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_AF);
	
	gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);	// TX
	gpio_init(GPIOA, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_11);	// RX
	
	can_parameter_struct			can_initTypeDef;
	can_struct_para_init(CAN_INIT_STRUCT, &can_initTypeDef);
	can_initTypeDef.prescaler				= 4;				// 1MHz
	can_initTypeDef.time_segment_1			= CAN_BT_BS1_7TQ;
	can_initTypeDef.time_segment_2			= CAN_BT_BS2_7TQ;
	can_initTypeDef.resync_jump_width		= CAN_BT_SJW_3TQ;
	can_initTypeDef.working_mode 			= CAN_NORMAL_MODE;	// 回环模式
	can_initTypeDef.auto_retrans			= ENABLE;			// 自动重传
	can_initTypeDef.auto_bus_off_recovery	= ENABLE;			// 离线自动恢复
	can_initTypeDef.auto_wake_up			= ENABLE;			// 自动唤醒
	can_initTypeDef.rec_fifo_overwrite		= DISABLE;			// 满时覆盖最旧报文
	can_initTypeDef.trans_fifo_order		= DISABLE;			// 发送按 ID 优先级
	can_initTypeDef.time_triggered			= DISABLE;
	
	if (can_init(CAN0, &can_initTypeDef) == SUCCESS) {
    // 初始化成功
	}
	
	foc_can_fliter_config();

}


// 过滤器配置: Filter0 掩码模式, 32bit, 全通过(mask=0), 挂到 FIFO0
void foc_can_fliter_config(void)
{
    can_filter_parameter_struct filter_struct;
    can_filter_init(&filter_struct);

    filter_struct.filter_number        = 0;                      // 使用过滤器组 0
    filter_struct.filter_mode          = CAN_FILTERMODE_MASK;    // 掩码模式
    filter_struct.filter_bits          = CAN_FILTERBITS_32BIT;   // 32bit 宽度
    filter_struct.filter_list_high     = 0x0000;                 // list=0
    filter_struct.filter_list_low      = 0x0000;
    filter_struct.filter_mask_high     = 0x0000;                 // mask=0 → 所有位都"不关心"
    filter_struct.filter_mask_low      = 0x0000;                 // → 任何 ID 的帧都通过
    filter_struct.filter_fifo_number   = CAN_FIFO0;              // 都进 fifo0
    filter_struct.filter_enable        = ENABLE;

    can_filter_init(&filter_struct);	
	
    can_interrupt_enable(CAN0, CAN_INT_RFNE0);
    nvic_irq_enable(USBD_LP_CAN0_RX0_IRQn, 1, 0);
}


// can.c 中新增
volatile uint8_t  can_ok = 0;
extern volatile uint8_t foc_mode;
extern volatile float foc_target_set;   // 上位机设的目标值
extern volatile uint8_t can_stop;       // 停止命令标志, Foc.c 轮询后切回待机

void USBD_LP_CAN0_RX0_IRQHandler(void)
{
    if (RESET != can_interrupt_flag_get(CAN0, CAN_INT_FLAG_RFL0))
    {
        can_interrupt_flag_clear(CAN0, CAN_INT_FLAG_RFL0);

        can_receive_message_struct rx_msg;
        can_message_receive(CAN0, CAN_FIFO0, &rx_msg);

        // 只处理 ID=0x101 的命令帧
        if (rx_msg.rx_sfid == 0x101 && rx_msg.rx_dlen >= 1)
        {
            uint8_t cmd = rx_msg.rx_data[0];
            switch (cmd)
            {
            case 0x01:  // 启动 FOC
                can_ok = 1;
                can_stop = 0;
                break;
            case 0x02:  // 停止 FOC: 只置标志, 由 Foc.c 负责切状态
                can_stop = 1;
                can_ok = 0;
                break;
            case 0x10:  // 设速度目标 (int16, 千度/秒×100)
                if (rx_msg.rx_dlen >= 3)
                {
                    int16_t tmp = (rx_msg.rx_data[1] << 8) | rx_msg.rx_data[2];
                    foc_target_set = (float)tmp / 100.0f;  // 还原为千度/秒
                    foc_mode = 2;
                }
                break;
            case 0x11:  // 设力矩目标 (int16, pu×10000)
                if (rx_msg.rx_dlen >= 3)
                {
                    int16_t tmp = (rx_msg.rx_data[1] << 8) | rx_msg.rx_data[2];
                    foc_target_set = (float)tmp / 10000.0f;  // 还原为 pu
                    foc_mode = 1;
                }
                break;
            case 0x12:  // 设控制模式
                if (rx_msg.rx_dlen >= 2)
                {
                    foc_mode = rx_msg.rx_data[1];
                }
                break;
            default:
                break;
            }
        }
    }
}





// can.c 新增
void CAN_Send_Status(float iq, float id, float speed)
{
    can_transmit_message_struct tx_msg;
    memset(&tx_msg, 0, sizeof(tx_msg));

    tx_msg.tx_sfid = 0x201;
    tx_msg.tx_ff   = CAN_FF_STANDARD;
    tx_msg.tx_ft   = CAN_FT_DATA;
    tx_msg.tx_dlen = 6;

    // iq: pu×10000 → int16
    int16_t iq_i16 = (int16_t)(iq * 10000.0f);
    int16_t id_i16 = (int16_t)(id * 10000.0f);
    int16_t spd_i16 = (int16_t)(speed * 100.0f);

    tx_msg.tx_data[0] = (iq_i16 >> 8) & 0xFF;
    tx_msg.tx_data[1] = iq_i16 & 0xFF;
    tx_msg.tx_data[2] = (id_i16 >> 8) & 0xFF;
    tx_msg.tx_data[3] = id_i16 & 0xFF;
    tx_msg.tx_data[4] = (spd_i16 >> 8) & 0xFF;
    tx_msg.tx_data[5] = spd_i16 & 0xFF;

    can_message_transmit(CAN0, &tx_msg);
}





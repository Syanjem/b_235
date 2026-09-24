#ifndef _CAN_H_
#define _CAN_H_

#include "gd32f30x_libopt.h"

#include <string.h>


void can0_init(void);
void foc_can_fliter_config(void);

extern can_receive_message_struct		rx_int_msg[3];
extern volatile uint8_t                 can0_rx_full_flag;
void USBD_LP_CAN0_RX0_IRQHandler(void);

extern can_transmit_message_struct		debug_code[2];
void can_debugcode_Init(can_transmit_message_struct** pdc);


extern can_transmit_message_struct		fault_code[20];

/* FOC 与 CAN 的耦合接口: can.c 只置标志, Foc.c 负责切 motorData 状态 */
extern volatile uint8_t  can_ok;          /* 1=收到启动命令, 0=未收到或已停止 */
extern volatile uint8_t  can_stop;        /* 1=收到停止命令, Foc.c 轮询后切回待机 */
extern volatile uint8_t  foc_mode;        /* 控制模式: 1=力矩 2=速度 3=位置 */
extern volatile float    foc_target_set;  /* 上位机设的目标值(千度/秒 或 pu) */
void CAN_Send_Status(float iq, float id, float speed);






#endif

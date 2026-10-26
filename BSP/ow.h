#ifndef _OW_H_
#define _OW_H_

#include "gd32f30x_libopt.h"

typedef enum {
    OW_IDLE,              // 空闲
    OW_RESET_LOW,         // 复位：拉低总线中
    OW_RESET_WAIT,        // 复位：已释放，等待存在脉冲采样点
    OW_WRITE_LOW,         // 写位：拉低中
    OW_WRITE_RELEASE,     // 写位：根据位值决定释放或保持
    OW_WRITE_END,         // 写位：时隙结束
    OW_READ_LOW,          // 读位：拉低中
    OW_READ_SAMPLE,       // 读位：到达采样点
    OW_READ_END,          // 读位：时隙结束
    OW_DONE,              // 一次业务序列完成
    OW_ERROR              // 出错（复位无应答）
} ow_state_t;

typedef enum {
    OW_IDLE,    // 空闲
    OW_WRITE,   // 写
    OW_READ     // 读
} ow_rw_t;

typedef enum {
    OW_WRITE_IDLE,    
    OW_WRITE_1_BEGIN,
    OW_WRITE_1_FIRST_LOW_BEGIN,
    OW_WRITE_1_FIRST_LOW_END,
    OW_WRITE_1_SECOND_UP_BEGIN,
    OW_WRITE_1_SECOND_UP_END,
    OW_WRITE_1_END,
    OW_WRITE_0_BEGIN,
    OW_WRITE_0_FIRST_LOW_BEGIN,
    OW_WRITE_0_FIRST_LOW_END,
    OW_WRITE_0_SECOND_UP_BEGIN,
    OW_WRITE_0_SECOND_UP_END,
    OW_WRITE_0_END,
    OW_WRITE_END        
} ow_write_state_t;

typedef enum {
    OW_WRITE_1_FIRST_LOW = 2,
    OW_WRITE_1_SECOND_UP = 68,
    OW_WRITE_0_FIRST_LOW = 65,
    OW_WRITE_0_SECOND_UP = 5
} ow_write_delay_t;

typedef struct {
    ow_write_state_t write_state;
    ow_write_delay_t write_delay_num;    
    uint16_t         write_delay_cnt;
} ow_write_para_s;

typedef enum {
    OW_READ_IDLE,    
    OW_READ_BEGIN,
    OW_READ_1_FIRST_LOW_BEGIN,
    OW_READ_1_FIRST_LOW_END,
    OW_READ_END,
} ow_read_state_t;

typedef enum {
    OW_READ_1_FIRST_LOW = 2,
    OW_READ_1_SECOND_UP = 8,
    OW_READ_0_WAIT      = 50
} ow_read_delay_t;

typedef struct {
    ow_read_state_t read_state;
    ow_read_delay_t read_delay_num;    
    uint16_t        read_delay_cnt;
} ow_read_para_s;

typedef struct {
    ow_rw_t rw_state;
    ow_write_para_s w_state;
    ow_read_para_s  r_state;
    uint32_t ow_nvic_cnt;
} ow_para_s;
extern ow_para_s g_ow;

extern uint16_t foc_cnt;



void MB1601B_write_bit_nvic(uint8_t bit)
{
    g_ow.ow_nvic_cnt++;
    static uint16_t delay = 0;

    if (bit)
    {
        switch (g_ow.w_state.write_state)
        {
            case OW_WRITE_1_BEGIN:
            {
                MB1601B_MODE_OUT();
                g_ow.w_state.write_state = OW_WRITE_1_FIRST_LOW_BEGIN;
                break;
            }
            case OW_WRITE_1_FIRST_LOW_BEGIN:
            {
                MB1601B_OUT_0();
                delay = g_ow.ow_nvic_cnt;
                g_ow.w_state.write_state = OW_WRITE_1_FIRST_LOW_END;
                break;
            }
            case OW_WRITE_1_FIRST_LOW_END:
            {
                if (g_ow.ow_nvic_cnt - delay == OW_WRITE_1_FIRST_LOW)
                {
                    delay = 0;
                    g_ow.w_state.write_state = OW_WRITE_1_SECOND_UP_BEGIN;
                }
                break;
            }
            case OW_WRITE_1_SECOND_UP_BEGIN:
            {
                MB1601B_OUT_1();
                delay = g_ow.ow_nvic_cnt;
                g_ow.w_state.write_state = OW_WRITE_1_SECOND_UP_END;
                break;
            }
            case OW_WRITE_1_SECOND_UP_END:
            {
                if (g_ow.ow_nvic_cnt - delay == OW_WRITE_1_SECOND_UP)
                {
                    delay = 0;
                    g_ow.w_state.write_state = OW_WRITE_1_END;
                }
                break;
            }
            default: break;
        }
    }
    else
    {
        switch (g_ow.w_state.write_state)
        {
            case OW_WRITE_0_BEGIN:
            {
                MB1601B_MODE_OUT();
                g_ow.w_state.write_state = OW_WRITE_0_FIRST_LOW_BEGIN;
                break;
            }
            case OW_WRITE_0_FIRST_LOW_BEGIN:
            {
                MB1601B_OUT_0();
                delay = g_ow.ow_nvic_cnt;
                g_ow.w_state.write_state = OW_WRITE_0_FIRST_LOW_END;
                break;
            }
            case OW_WRITE_0_FIRST_LOW_END:
            {
                if (g_ow.ow_nvic_cnt - delay == OW_WRITE_0_FIRST_LOW)
                {
                    delay = 0;
                    g_ow.w_state.write_state = OW_WRITE_0_SECOND_UP_BEGIN;
                }
                break;
            }
            case OW_WRITE_0_SECOND_UP_BEGIN:
            {
                MB1601B_OUT_1();
                delay = g_ow.ow_nvic_cnt;
                g_ow.w_state.write_state = OW_WRITE_0_SECOND_UP_END;
                break;
            }
            case OW_WRITE_0_SECOND_UP_END:
            {
                if (g_ow.ow_nvic_cnt - delay == OW_WRITE_0_SECOND_UP)
                {
                    delay = 0;
                    g_ow.w_state.write_state = OW_WRITE_0_END;
                }
                break;
            }
            default: break;
        }
    }
}



















ow_state_t ow_state;
uint32_t   ow_tick_start;      // 当前阶段的起始硬件计数值
uint8_t    ow_byte_index;      // 当前第几个字节
uint8_t    ow_bit_index;       // 当前字节第几位
uint8_t    ow_current_byte;    // 正在写/读的字节
uint8_t    ow_buffer[2];       // 读回的 lt、ht
uint8_t    ow_cmd_phase;       // 0=写0xCC, 1=写0xBE, 2=读lt, 3=读ht










#endif

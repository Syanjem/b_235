#include "ow.h"

void ow_tick(void) {
    uint32_t now = OW_TIM->CNT;  // 读硬件计数器，不用中断次数累加

    switch (ow_state) {
    case OW_RESET_LOW:
        if (now - ow_tick_start >= RESET_LOW_TICKS) {
            release_bus();
            ow_tick_start = now;
            ow_state = OW_RESET_WAIT;
        }
        break;

    case OW_RESET_WAIT:
        if (now - ow_tick_start >= RESET_SAMPLE_TICKS) {
            if (!read_bus()) {           // 存在脉冲为低表示有应答
                ow_tick_start = now;
                ow_state = OW_RESET_DONE;
            } else {
                ow_state = OW_ERROR;     // 无应答
            }
        }
        break;

    case OW_WRITE_LOW:
        if (now - ow_tick_start >= WRITE_LOW_TICKS) {
            if (ow_current_byte & 0x01) release_bus();  // 写1释放
            // 写0则继续保持低
            ow_state = OW_WRITE_END;
        }
        break;

    case OW_WRITE_END:
        if (now - ow_tick_start >= WRITE_SLOT_TICKS) {
            ow_bit_index++;
            if (ow_bit_index >= 8) {
                ow_bit_index = 0;
                ow_advance_byte_phase();   // 切换到下一个字节/阶段
            } else {
                ow_current_byte >>= 1;
                start_write_bit();          // 拉低，进入 OW_WRITE_LOW
            }
        }
        break;

    case OW_READ_LOW:
        if (now - ow_tick_start >= READ_LOW_TICKS) {
            release_bus();
            ow_state = OW_READ_SAMPLE;
        }
        break;

    case OW_READ_SAMPLE:
        if (now - ow_tick_start >= READ_SAMPLE_TICKS) {
            if (read_bus()) ow_current_byte |= (1 << ow_bit_index);
            ow_state = OW_READ_END;
        }
        break;

    case OW_READ_END:
        if (now - ow_tick_start >= READ_SLOT_TICKS) {
            ow_bit_index++;
            if (ow_bit_index >= 8) {
                ow_bit_index = 0;
                ow_advance_byte_phase();
            } else {
                start_read_bit();
            }
        }
        break;

    default: break;  // IDLE / DONE / ERROR 不动作
    }
}








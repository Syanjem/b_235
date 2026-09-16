#ifndef SYS_TICK_H
#define SYS_TICK_H

#include <stdint.h>

/* configure systick */
void SysTick_Init(void);
/* delay a time in milliseconds */
void delay_1ms(uint32_t count);
/* delay decrement */
void delay_decrement(void);

void delay_us(uint32_t nus);  // 新增

uint32_t get_ms_ticks(void);  // 毫秒时间戳(开机起累计)

#endif /* SYS_TICK_H */

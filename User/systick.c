#include "gd32f30x.h"
#include "systick.h"

volatile static uint32_t delay;

static uint32_t g_fac_us = 0;  // 每微秒的计数值

volatile static uint32_t g_ms_ticks = 0;  // 毫秒时间戳(开机起累计)

/*!
    \brief      configure systick
    \param[in]  none
    \param[out] none
    \retval     none
*/
void SysTick_Init(void)
{
    g_fac_us = SystemCoreClock / 1000000U;
	
    /* setup systick timer for 1000Hz interrupts */
    if (SysTick_Config(SystemCoreClock / 1000U)){
        /* capture error */
        while (1){
        }
    }
    /* configure the systick handler priority */
    NVIC_SetPriority(SysTick_IRQn, 0x02U);
}

/*!
    \brief      delay a time in milliseconds
    \param[in]  count: count in milliseconds
    \param[out] none
    \retval     none
*/
void delay_1ms(uint32_t count)
{
    delay = count;

    while(0U != delay){
    }
}

/*!
    \brief      delay decrement
    \param[in]  none
    \param[out] none
    \retval     none
*/
void delay_decrement(void)
{
    g_ms_ticks++;               // 每1ms由SysTick_Handler调用, 累计时间戳
    if (0U != delay){
        delay--;
    }
}

/*!
    \brief      获取开机以来的毫秒时间戳
    \retval     毫秒数(uint32_t, 无符号减法可安全处理约49.7天的回绕)
*/
uint32_t get_ms_ticks(void)
{
    return g_ms_ticks;
}


/*!
    \brief      微秒级阻塞延时 (使用SysTick硬件计数器)
    \param[in]  nus: 需要延时的微秒数
    \param[out] none
    \retval     none
    \note       此函数会短暂占用SysTick硬件，但不会影响原有的1ms中断功能
                单次最大延时: 0xFFFFFF / g_fac_us (微秒)
                例如: 108MHz下最大约155ms
*/
void delay_us(uint32_t nus)
{
    uint32_t temp;
    
    /* 关闭SysTick中断，避免在延时过程中被打断 */
    SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
    
    /* 设置重装载值 */
    SysTick->LOAD = nus * g_fac_us;
    /* 清空当前值，确保从0开始 */
    SysTick->VAL = 0U;
    /* 使能计数器，使用内核时钟，不使能中断 */
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    
    /* 等待计数完成 */
    do {
        temp = SysTick->CTRL;
    } while ((temp & SysTick_CTRL_ENABLE_Msk) && !(temp & SysTick_CTRL_COUNTFLAG_Msk));
    
    /* 关闭计数器 */
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    /* 清空当前值 */
    SysTick->VAL = 0U;
    
    /* 重新使能中断，恢复原来的中断功能 */
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;

    /* 还原1ms节拍: 恢复重装载值并重新使能计数器,
       否则计数器停留在关闭状态, 之后的 delay_1ms 将永久卡死 */
    SysTick->LOAD = SystemCoreClock / 1000U;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

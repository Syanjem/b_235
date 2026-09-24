#include "FPC_jr.h"

void fpc_init(void)
{
    rcu_periph_clock_enable(RCU_GPIOB);

    gpio_init(FPC_POTR, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, FPC_PIN);

}













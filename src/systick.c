#include "stm32f4xx.h"

#define SYSTICK_LOAD_VAL            16000   // 1 ms
#define CTRL_ENABLE_MASK            (1U<<0)
#define CTRL_CLKSOURCE_MASK         (1U<<2)
#define CTRL_COUNTFLAG_MASK         (1U<<16)

void systickDelayMs(int delay){
    SysTick->LOAD = SYSTICK_LOAD_VAL;
    SysTick->VAL = 0;
    SysTick->CTRL |= CTRL_ENABLE_MASK | CTRL_CLKSOURCE_MASK;
    for(int i = 0; i < delay; i++){
        while(!(SysTick->CTRL & CTRL_COUNTFLAG_MASK));
    }
    SysTick->CTRL &= ~CTRL_ENABLE_MASK;   
}
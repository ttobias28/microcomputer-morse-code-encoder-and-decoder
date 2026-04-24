/*
 * file name: timers.c
 *
 * timer math (all timers on APB1, running at SYSCLK = 4 MHz):
 *   timer period = (PSC + 1) * (ARR + 1) / SYSCLK
 */

#include "morse.h"

/* ----------------------------------------------------------
 TIM2 = 200 ms PERIODIC INTERRUPT (encode output engine)
 
 this is the "heartbeat" of encode mode.  every tick, the
 TIM2 ISR in stm32l4xx_it.c advances the encode FSM one step:
------------------------------------------------------------- */
void TIM2_Init(void)
{
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;

    // set prescaler, divides 4 MHz down to 1000 Hz
    TIM2->PSC = TIM2_PSC;   /* 3999 */

    // set auto-reload, timer counts 0..199 then overflows
    TIM2->ARR = TIM2_ARR;   /* 199 */

    // enable the update interrupt (fires on each overflow)
    TIM2->DIER |= TIM_DIER_UIE; 

    // clear any stale pending interrupt flag
    TIM2->SR &= ~TIM_SR_UIF;

    // tell NVIC to accept TIM2 interrupts.
    NVIC->ISER[0] = (1U << (TIM2_IRQn & 0x1F));

    // set priority lower than EXTI
    NVIC_SetPriority(TIM2_IRQn, 2);

}

void TIM2_Start(void)
{
    TIM2->SR  &= ~TIM_SR_UIF;   // clear any pending flag before starting
    TIM2->CNT  = 0;             // reset counter so first tick is a full period
    TIM2->CR1 |= TIM_CR1_CEN; 
}

void TIM2_Stop(void)
{
    TIM2->CR1 &= ~TIM_CR1_CEN; // clear CEN to freeze the counter 
    TIM2->CNT  = 0;
}


 //TIM3 = FREE-RUNNING MILLISECOND COUNTER (decode input)
void TIM3_Init(void)
{
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM3EN;

    // 1 ms per count
    TIM3->PSC = TIM3_PSC;   // 3999
    TIM3->ARR = TIM3_ARR;   // 0xFFFF

    // no interrupt needed on TIM3

    // clear counter, but don't start yet
    TIM3->CNT = 0;
}

void TIM3_Reset_And_Start(void)
{
    TIM3->CR1 &= ~TIM_CR1_CEN;  // stop first (safe)
    TIM3->CNT  = 0;              // reset to zero 
    TIM3->CR1 |= TIM_CR1_CEN;   // start counting
}

void TIM3_Stop(void)
{
    TIM3->CR1 &= ~TIM_CR1_CEN;
}

// read the current counter value as milliseconds
uint32_t TIM3_Read_MS(void)
{
    return TIM3->CNT;
}


/* ----------------------------------------------------------
TIM6 = 600 ms ONE-SHOT TIMEOUT (decode letter trigger)

after every button release in decode mode, we restart TIM6.
TIM6 is a basic timer (no capture/compare)

One Pulse Mode: timer stops itself after one overflow
-------------------------------------------------------------- */
void TIM6_Init(void)
{
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM6EN;

    TIM6->PSC = TIM6_PSC;   // 3999, 1 ms resolution 
    TIM6->ARR = TIM6_ARR;   // 599, 600 ms period  

    // one-pulse mode: timer stops automatically after one overflow.
    TIM6->CR1 |= TIM_CR1_OPM;

    // enable update interrupt
    TIM6->DIER |= TIM_DIER_UIE;

    // clear pending flag
    TIM6->SR &= ~TIM_SR_UIF;

    // enable in NVIC
    NVIC->ISER[1] = (1U << ((TIM6_DAC_IRQn) & 0x1F));
    NVIC_SetPriority(TIM6_DAC_IRQn, 2);
}

// restart TIM6 one-shot countdown from zero
void TIM6_Start_OneShot(void)
{
    TIM6->CR1 &= ~TIM_CR1_CEN;
    TIM6->SR  &= ~TIM_SR_UIF; 
    TIM6->CNT  = 0;
    TIM6->CR1 |= TIM_CR1_CEN; 
}

void TIM6_Stop(void)
{
    TIM6->CR1 &= ~TIM_CR1_CEN;
    TIM6->SR  &= ~TIM_SR_UIF;
}
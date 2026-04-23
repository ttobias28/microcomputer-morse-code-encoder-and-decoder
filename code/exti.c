/*
 * file name: exti.c
 *
 * configures external interrupts.
 *
 * SW4 = PC13 = blue NUCLEO button
 *   active LOW, pull-up
 *   both edges: falling = pressed, rising = released
 *   EXTI line 13 -> EXTI15_10_IRQHandler (IRQn = 40)
 *
 * mode toggle is handled via the '*' key on the keypad in main.c
 * no second EXTI needed.
 */

#include "morse.h"

void EXTI_SW4_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    /* PC13 -> EXTI line 13: EXTICR[3] bits [7:4] = 0x2 */
    SYSCFG->EXTICR[3] &= ~(0xFU << 4);
    SYSCFG->EXTICR[3] |=  (0x2U << 4);

    EXTI->RTSR1 |= (1U << 13);
    EXTI->FTSR1 |= (1U << 13);
    EXTI->IMR1  |= (1U << 13);

    NVIC->ISER[1] = (1U << (EXTI15_10_IRQn & 0x1F));
    NVIC_SetPriority(EXTI15_10_IRQn, 0);
}

/* EXTI_SW5_Init kept as empty stub so main.c compiles without changes */
void EXTI_SW5_Init(void)
{
    /* mode toggle is now handled by '*' key in keypad - no EXTI needed */
}
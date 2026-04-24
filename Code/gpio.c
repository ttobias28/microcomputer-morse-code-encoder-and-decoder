/*
 * file name: gpio.c
 *
 * initialises every GPIO pin used by the project.
 */

#include "morse.h"

#define SET_OUTPUT(PORT, PIN)                      \
    do {                                           \
        uint32_t _t;                               \
        _t = (PORT)->MODER;                        \
        _t &= ~(0x3U << (2*(PIN)));                \
        _t |=  (0x1U << (2*(PIN)));                \
        (PORT)->MODER = _t;                        \
        (PORT)->OTYPER &= ~(1U << (PIN));          \
        _t = (PORT)->PUPDR;                        \
        _t &= ~(0x3U << (2*(PIN)));                \
        (PORT)->PUPDR = _t;                        \
    } while(0)

#define SET_INPUT_PULLDOWN(PORT, PIN)              \
    do {                                           \
        uint32_t _t;                               \
        _t = (PORT)->MODER;                        \
        _t &= ~(0x3U << (2*(PIN)));                \
        (PORT)->MODER = _t;                        \
        (PORT)->OTYPER &= ~(1U << (PIN));          \
        _t = (PORT)->PUPDR;                        \
        _t &= ~(0x3U << (2*(PIN)));                \
        _t |=  (0x2U << (2*(PIN)));                \
        (PORT)->PUPDR = _t;                        \
    } while(0)

#define SET_INPUT_PULLUP(PORT, PIN)                \
    do {                                           \
        uint32_t _t;                               \
        _t = (PORT)->MODER;                        \
        _t &= ~(0x3U << (2*(PIN)));                \
        (PORT)->MODER = _t;                        \
        (PORT)->OTYPER &= ~(1U << (PIN));          \
        _t = (PORT)->PUPDR;                        \
        _t &= ~(0x3U << (2*(PIN)));                \
        _t |=  (0x1U << (2*(PIN)));                \
        (PORT)->PUPDR = _t;                        \
    } while(0)

void GPIO_Init_All(void)
{
    // enable clocks
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN   /* PA: LEDs, shift clock, LCD latch */
                  | RCC_AHB2ENR_GPIOBEN   /* PB: keypad, shift data, PB8 toggle */
                  | RCC_AHB2ENR_GPIOCEN;  /* PC: LEDs, buzzer, 7-seg latch, PC13 */

    // LEDs: active HIGH push-pull outputs
    SET_OUTPUT(GPIOA, 1);  
    SET_OUTPUT(GPIOA, 0);  
    SET_OUTPUT(GPIOC, 7);  
    SET_OUTPUT(GPIOC, 8);  

    // start with all LEDs off
    GPIOA->ODR &= ~((1U<<1)|(1U<<0));
    GPIOC->ODR &= ~((1U<<7)|(1U<<8));

    // buzzer: push-pull output, start silent
    SET_OUTPUT(GPIOC, 9);
    GPIOC->ODR &= ~(1U << 9);

    // shift register pins 
    SET_OUTPUT(GPIOB, 5);   /* PB5  - serial data (shared) */
    SET_OUTPUT(GPIOA, 5);   /* PA5  - shift clock (shared) */
    SET_OUTPUT(GPIOC, 10);  /* PC10 - 7-seg latch */
    SET_OUTPUT(GPIOA, 10);  /* PA10 - LCD latch */

    // PC13 = blue NUCLEO button = dot/dash input in decode mode
		// active LOW - needs pull-up 
    SET_INPUT_PULLUP(GPIOC, 13);

    // PB8 = mode toggle button (redundant ?)
    SET_INPUT_PULLDOWN(GPIOB, 8);

}
/*
 * file name: stm32l4xx_it.c
 *
 * ISRs for the Morse Code project.
 *
 *   TIM2_IRQHandler       - 200 ms encode output tick
 *   TIM6_DAC_IRQHandler   - 600 ms decode timeout
 *   EXTI15_10_IRQHandler  - PC13 blue button (dot/dash input, decode mode)
 *
 * mode toggle: handled by '*' key on keypad in main loop - no ISR needed.
 */

#include "stm32l4xx_hal.h"
#include "morse.h"

void NMI_Handler(void)      { while(1){} }
void HardFault_Handler(void){ while(1){} }
void SysTick_Handler(void)  { HAL_IncTick(); }

void TIM2_IRQHandler(void)
{
    if(!(TIM2->SR & TIM_SR_UIF)) return;
    TIM2->SR &= ~TIM_SR_UIF;

    if(g_mode != MODE_ENCODE) return;

    g_tim2_ticks++;

    switch(g_enc_state)
    {
        case ENC_IDLE:
            break;

        case ENC_START_SYMBOL:
        {
            char sym = g_enc_pattern[g_enc_pattern_idx];
            if(sym == '\0'){
                BUZZER_OFF(); LED0_OFF();
                g_enc_symbol_ticks = LETTER_GAP_TICKS;
                g_enc_state = ENC_INTER_LETTER;
            } else {
                BUZZER_ON(); LED0_ON();
                g_enc_symbol_ticks = (sym == '.') ? DOT_TICKS : DASH_TICKS;
                g_enc_state = ENC_SOUND_ON;
            }
            break;
        }

        case ENC_SOUND_ON:
            g_enc_symbol_ticks--;
            if(g_enc_symbol_ticks <= 0){
                BUZZER_OFF(); LED0_OFF();
                g_enc_symbol_ticks = SYMBOL_GAP_TICKS;
                g_enc_state = ENC_SOUND_OFF;
            }
            break;

        case ENC_SOUND_OFF:
            g_enc_symbol_ticks--;
            if(g_enc_symbol_ticks <= 0){
                g_enc_pattern_idx++;
                g_enc_state = ENC_START_SYMBOL;
            }
            break;

        case ENC_INTER_LETTER:
            g_enc_symbol_ticks--;
            if(g_enc_symbol_ticks <= 0)
                g_enc_state = ENC_IDLE;
            break;

        case ENC_INTER_WORD:
            g_enc_symbol_ticks--;
            if(g_enc_symbol_ticks <= 0)
                g_enc_state = ENC_IDLE;
            break;

        case ENC_ERROR:
            g_enc_state = ENC_IDLE;
            break;

        default:
            break;
    }
}

void TIM6_DAC_IRQHandler(void)
{
    if(!(TIM6->SR & TIM_SR_UIF)) return;
    TIM6->SR &= ~TIM_SR_UIF;

    if(g_mode != MODE_DECODE) return;

    g_flag_tim6_fired = 1;
}

/* PC13 blue button - active LOW
 * falling = pressed, rising = released */
void EXTI15_10_IRQHandler(void)
{
    if(!(EXTI->PR1 & (1U << 13))) return;
    EXTI->PR1 |= (1U << 13);

    if(g_mode != MODE_DECODE) return;

    if(!(GPIOC->IDR & (1U << 13)))
    {
        /* falling edge = pressed */
        TIM3_Reset_And_Start();
        TIM6_Stop();
        g_flag_tim6_fired  = 0;
        g_flag_sw4_pressed = 1;
        g_dec_state = DEC_BUTTON_DOWN;
    }
    else
    {
        /* rising edge = released */
        TIM3_Stop();
        g_sw4_press_duration_ms = TIM3_Read_MS();
        g_flag_sw4_released = 1;
        TIM6_Start_OneShot();
    }
}
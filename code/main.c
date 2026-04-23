/*
 * main.c
 *
 * Morse Code Encoder/Decoder
 * Hardware: STM32 NUCLEO-L476RG + EDUBase V2
 * Course:   ECE 3130
 *
 * buttons:
 *   PC13 = blue NUCLEO button = dot/dash input in decode mode
 *   '*'  = keypad key         = mode toggle (encode <-> decode)
 *   '#'  = keypad key         = word gap in encode mode
 */

#include "stm32l4xx_hal.h"
#include "morse.h"

volatile SystemMode  g_mode         = MODE_ENCODE;
volatile EncodeState g_enc_state    = ENC_IDLE;
volatile DecodeState g_dec_state    = DEC_IDLE;
volatile uint32_t    g_tim2_ticks   = 0;
volatile char g_enc_pattern[8]      = {0};
volatile int  g_enc_pattern_idx     = 0;
volatile int  g_enc_symbol_ticks    = 0;
volatile char g_queue[QUEUE_LEN]    = {0};
volatile int  g_queue_head          = 0;
volatile int  g_queue_tail          = 0;
volatile char g_dec_buf[DECODE_BUF_LEN + 1] = {0};
volatile int  g_dec_buf_idx         = 0;
volatile uint8_t  g_flag_sw4_pressed      = 0;
volatile uint8_t  g_flag_sw4_released     = 0;
volatile uint8_t  g_flag_sw5_pressed      = 0;
volatile uint8_t  g_flag_tim6_fired       = 0;
volatile uint32_t g_sw4_press_duration_ms = 0;

int main(void)
{
    HAL_Init();
    System_Clock_Config();

    GPIO_Init_All();
    Keypad_Init();
    LCD_Init();
    Seg7_Init();
    TIM2_Init();
    TIM3_Init();
    TIM6_Init();
    EXTI_SW4_Init();
    EXTI_SW5_Init();

    LCD_Clear();
    LCD_Write_String("Morse Encoder");
    LCD_Set_Cursor(1, 0);
    LCD_Write_String("*=mode 1=EFG...");

    TIM2_Start();

    while(1)
    {
        /* always scan keypad regardless of mode so '*' is detected
         * in both encode and decode mode. Encode_Run also calls
         * Keypad_Read internally, so in encode mode we call it here
         * only to catch '*' for mode toggle — Encode_Run handles the
         * rest. In decode mode this is the only keypad scan. */
        if(g_mode == MODE_DECODE)
        {
            char k = Keypad_Read();
            if(k != 0 && g_flag_sw5_pressed == 0)
            {
                /* in decode mode only '*' does anything via the flag
                 * set inside Keypad_Read — ignore all other keys */
            }
        }

        /* mode toggle — set by '*' key inside Keypad_Read() */
        if(g_flag_sw5_pressed)
        {
            g_flag_sw5_pressed = 0;
            Reset_All_State();

            if(g_mode == MODE_ENCODE)
            {
                g_mode = MODE_DECODE;
                LCD_Clear();
                LCD_Write_String("Morse Decoder");
                LCD_Set_Cursor(1, 0);
                LCD_Write_String("Tap blue button");
            }
            else
            {
                g_mode = MODE_ENCODE;
                LCD_Clear();
                LCD_Write_String("Morse Encoder");
                LCD_Set_Cursor(1, 0);
                LCD_Write_String("*=mode 1=EFG...");
                TIM2_Start();
            }
        }

        if(g_mode == MODE_ENCODE)
            Encode_Run();
        else
            Decode_Run();
    }
}
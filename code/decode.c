/*
 * file name: decode.c
 *
 * decode mode logic - runs from the main loop (not an ISR).
 * consumes flags set by EXTI15_10_IRQHandler and TIM6_DAC_IRQHandler.
 *
 * LCD layout:
 *   line 1: "Morse Decoder" on startup, then decoded letters scrolling.
 *            cleared and replaced on first successfully decoded letter.
 *   line 2: dots/dashes accumulating as user taps, cleared after decode.
 */

#include "morse.h"

static char decoded_line[17] = {0};
static int  decoded_col  = 0;
static int  first_decode = 1;   /* 1 = no letter decoded yet */

void Decode_Init(void)
{
    g_dec_state             = DEC_IDLE;
    g_dec_buf_idx           = 0;
    g_dec_buf[0]            = '\0';
    g_flag_sw4_pressed      = 0;
    g_flag_sw4_released     = 0;
    g_flag_tim6_fired       = 0;
    g_sw4_press_duration_ms = 0;
    decoded_col  = 0;
    first_decode = 1;
    memset(decoded_line, ' ', 16);
    decoded_line[16] = '\0';
}

void Decode_Run(void)
{
    /* SW4 pressed (blue button, falling edge detected in ISR) */
    if(g_flag_sw4_pressed)
    {
        g_flag_sw4_pressed = 0;
        g_dec_state = DEC_BUTTON_DOWN;
        LED0_ON();

        /* show accumulating pattern with underscore for in-progress press */
        char disp[17];
        int i;
        for(i = 0; i < g_dec_buf_idx && i < 15; i++)
            disp[i] = g_dec_buf[i];
        disp[i++] = '_';
        while(i < 16) disp[i++] = ' ';
        disp[16] = '\0';
        LCD_Set_Cursor(1, 0);
        LCD_Write_String(disp);
    }

    /* SW4 released (rising edge detected in ISR) */
    if(g_flag_sw4_released)
    {
        g_flag_sw4_released = 0;
        LED0_OFF();

        uint32_t dur = g_sw4_press_duration_ms;
        char symbol = (dur < DOT_DASH_THRESHOLD_MS) ? '.' : '-';

        g_dec_state = (symbol == '.') ? DEC_APPEND_DOT : DEC_APPEND_DASH;

        if(g_dec_buf_idx < DECODE_BUF_LEN)
        {
            g_dec_buf[g_dec_buf_idx] = symbol;
            g_dec_buf_idx++;
            g_dec_buf[g_dec_buf_idx] = '\0';
        }

        /* show accumulated pattern on line 2 */
        char disp[17];
        int i;
        for(i = 0; i < g_dec_buf_idx && i < 16; i++)
            disp[i] = g_dec_buf[i];
        while(i < 16) disp[i++] = ' ';
        disp[16] = '\0';
        LCD_Set_Cursor(1, 0);
        LCD_Write_String(disp);

        /* short beep to confirm symbol received */
        BUZZER_ON();
        Delay(20);
        BUZZER_OFF();

        g_dec_state = DEC_WAIT_NEXT;
    }

    /* TIM6 fired: 600 ms silence - decode the buffer */
    if(g_flag_tim6_fired)
    {
        g_flag_tim6_fired = 0;
        g_dec_state = DEC_DECODE_LETTER;

        if(g_dec_buf_idx == 0)
        {
            /* empty buffer - word gap, insert space */
            if(decoded_col > 0 && decoded_line[decoded_col-1] != ' ')
            {
                if(!first_decode && decoded_col < 16)
                {
                    decoded_line[decoded_col] = ' ';
                    decoded_col++;
                }
            }
            g_dec_state = DEC_IDLE;
        }
        else
        {
            char found = Morse_Decode((const char*)g_dec_buf);

            if(found == '?')
            {
                g_dec_state = DEC_ERROR;
                LCD_Set_Cursor(1, 0);
                LCD_Write_String("? Unknown Morse ");
                Buzzer_Beep(100);
                Buzzer_Beep(100);
            }
            else
            {
                g_dec_state = DEC_WRITE_LCD;

                if(first_decode)
                {
                    /* first letter decoded - clear "Morse Decoder" from line 1 */
                    first_decode = 0;
                    memset(decoded_line, ' ', 16);
                    decoded_line[16] = '\0';
                    decoded_col = 0;
                }

                /* scroll if needed */
                if(decoded_col >= 16)
                {
                    for(int i = 0; i < 15; i++)
                        decoded_line[i] = decoded_line[i+1];
                    decoded_line[15] = found;
                }
                else
                {
                    decoded_line[decoded_col] = found;
                    decoded_col++;
                }
                decoded_line[16] = '\0';

                LCD_Set_Cursor(0, 0);
                LCD_Write_String(decoded_line);
                Seg7_Display_Char(found);
                Buzzer_Beep(100);
            }

            /* clear buffer and line 2 for next letter */
            g_dec_buf_idx = 0;
            g_dec_buf[0]  = '\0';
            LCD_Set_Cursor(1, 0);
            LCD_Write_String("                ");

            g_dec_state = DEC_IDLE;
        }
    }
}
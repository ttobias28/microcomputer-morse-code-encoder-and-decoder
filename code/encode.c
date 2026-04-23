/*
 * file name: encode.c
 *
 * encode mode logic - runs from the main loop (not an ISR).
 *
 * LCD layout:
 *   line 1: "Morse Encoder" on startup, then confirmed typed characters
 *            scrolling left as the user types. cleared and replaced on
 *            first keypress.
 *   line 2: "X: .-."  format for BOTH preview (mid-tap) and playing.
 *            cleared before each new letter is shown.
 *
 * 7-segment: always shows the most recently selected letter,
 *            whether preview or confirmed and playing.
 *
 * multi-tap groups:
 *   1=EFG  2=HIJ  3=KLM  4=NOP  5=QRS  6=TUV  7=WXYZ
 *   A B C D 0 8 9 = direct
 *   * = mode toggle     # = word gap
 */

#include "morse.h"

static char lcd_line1[17] = {0};
static int  lcd_col       = 0;
static char last_preview  = 0;
static int  first_key     = 1;   /* 1 = no key typed yet, startup msg showing */

void Encode_Init(void)
{
    g_enc_state        = ENC_IDLE;
    g_enc_pattern[0]   = '\0';
    g_enc_pattern_idx  = 0;
    g_enc_symbol_ticks = 0;
    g_queue_head = 0;
    g_queue_tail = 0;
    lcd_col      = 0;
    last_preview = 0;
    first_key    = 1;
    memset(lcd_line1, ' ', 16);
    lcd_line1[16] = '\0';
}

void Encode_Queue_Push(char c)
{
    int next = (g_queue_head + 1) % QUEUE_LEN;
    if(next == g_queue_tail) return;
    g_queue[g_queue_head] = c;
    g_queue_head = next;
}

static char queue_pop(void)
{
    if(g_queue_head == g_queue_tail) return '\0';
    char c = g_queue[g_queue_tail];
    g_queue_tail = (g_queue_tail + 1) % QUEUE_LEN;
    return c;
}

static int queue_empty(void)
{
    return (g_queue_head == g_queue_tail);
}

/*
 * clear_line2
 * wipe line 2 before writing a new pattern so no leftover dots/dashes show.
 */
static void clear_line2(void)
{
    LCD_Set_Cursor(1, 0);
    LCD_Write_String("                ");
}

/*
 * show_letter_on_display
 * writes "X: .-." to LCD line 2 and the letter to the 7-segment.
 * clears line 2 first so no stale characters remain.
 * used for BOTH live preview and confirmed-playing states.
 */
static void show_letter_on_display(char c)
{
    const char *pat = Morse_Encode(c);
    if(!pat) return;

    char disp[17];
    int j = 0;
    disp[j++] = c;
    disp[j++] = ':';
    disp[j++] = ' ';
    for(int k = 0; pat[k] && j < 16; k++, j++)
        disp[j] = pat[k];
    while(j < 16) disp[j++] = ' ';
    disp[16] = '\0';

    LCD_Set_Cursor(1, 0);
    LCD_Write_String(disp);
    Seg7_Display_Char(c);
}

/*
 * lcd_add_char
 * add a confirmed character to LCD line 1.
 * on first keypress, clears "Morse Encoder" and starts the typed scroll.
 */
static void lcd_add_char(char c)
{
    if(first_key)
    {
        /* first character typed - clear line 1 and start fresh */
        first_key = 0;
        memset(lcd_line1, ' ', 16);
        lcd_line1[16] = '\0';
        lcd_col = 0;
    }

    if(lcd_col >= 16){
        /* scroll left */
        for(int i = 0; i < 15; i++) lcd_line1[i] = lcd_line1[i+1];
        lcd_line1[15] = c;
    } else {
        lcd_line1[lcd_col] = c;
        lcd_col++;
    }
    lcd_line1[16] = '\0';
    LCD_Set_Cursor(0, 0);
    LCD_Write_String(lcd_line1);
}

void Encode_Run(void)
{
    /* -- step 1: keypad scan ------------------------------ */
    char confirmed = Keypad_Read();
    char preview   = Keypad_GetPreview();

    /* show live preview on line 2 + 7-seg whenever it changes */
    if(preview != 0 && preview != last_preview)
    {
        last_preview = preview;
        show_letter_on_display(preview);
    }
    else if(preview == 0)
    {
        last_preview = 0;
    }

    if(confirmed != 0)
    {
        if(confirmed == '#')
        {
            Encode_Queue_Push('#');
        }
        else if(Morse_Encode(confirmed) != NULL)
        {
            Encode_Queue_Push(confirmed);
            lcd_add_char(confirmed);
        }
        else
        {
            clear_line2();
            LCD_Set_Cursor(1, 0);
            LCD_Write_String("? No Morse code ");
            Buzzer_Beep(50);
        }
    }

    /* -- step 2: feed TIM2 output engine ----------------- */
    if(g_enc_state == ENC_IDLE && !queue_empty())
    {
        char c = queue_pop();

        if(c == '#')
        {
            g_enc_symbol_ticks = WORD_GAP_TICKS;
            g_enc_state = ENC_INTER_WORD;
            clear_line2();
            LCD_Set_Cursor(1, 0);
            LCD_Write_String("[word gap]      ");
        }
        else
        {
            const char *pat = Morse_Encode(c);
            if(pat == NULL)
            {
                g_enc_state = ENC_ERROR;
            }
            else
            {
                int i = 0;
                while(pat[i] && i < 7){ g_enc_pattern[i] = pat[i]; i++; }
                g_enc_pattern[i] = '\0';
                g_enc_pattern_idx  = 0;
                g_enc_symbol_ticks = 0;
                g_enc_state = ENC_START_SYMBOL;

                /* clear line 2 then show "X: .-." */
                show_letter_on_display(c);
            }
        }
    }
}
/*
 * file name: morse.h
 *
 * central header for the Morse Code Encoder/Decoder project.
 *
 * button assignments:
 *   PC13 = blue NUCLEO button = dot/dash input in decode mode
 */

#ifndef MORSE_H
#define MORSE_H

#include "stm32l4xx.h"
#include <stdint.h>
#include <string.h>

// system clock 4MHz
#define SYSCLK_HZ   4000000UL
 
 /*-------------------------------------- 
 morse timing
 international Morse standard: 1 unit = 200ms
			dot = 1 unit
			dash = 3 units
			intra-symbol = 1 unit
			inter-letter = 2 units
			inter-word = 7 units
---------------------------------------- */
#define DOT_TICKS        1
#define DASH_TICKS       3
#define SYMBOL_GAP_TICKS 1
#define LETTER_GAP_TICKS 3
#define WORD_GAP_TICKS   7


 // TIM2 = 200ms period
#define TIM2_PSC  3999
#define TIM2_ARR   199

// TIM3 = free-running, 1 count = 1ms 
#define TIM3_PSC  3999
#define TIM3_ARR  0xFFFF

// dot/dash threshold: press < 300 ms = dot, >= 300 ms = dash
#define DOT_DASH_THRESHOLD_MS  300

 // TIM6 = 600ms one-shot decode timeout
#define TIM6_PSC  3999
#define TIM6_ARR   599

 // multi-tap timeout for encode mode
 // if user stops tapping for this many ms, confirm letter
#define MULTITAP_TIMEOUT_LOOPS  1500   // ~1 second at typical main-loop speed


// LEDs (all implemented in case they are used)
#define LED0_PORT  GPIOA
#define LED0_PIN   1        /* PA1 */
#define LED1_PORT  GPIOA
#define LED1_PIN   0        /* PA0 */
#define LED2_PORT  GPIOC
#define LED2_PIN   7        /* PC7 */
#define LED3_PORT  GPIOC
#define LED3_PIN   8        /* PC8 */

#define LED0_ON()   (LED0_PORT->ODR |=  (1U << LED0_PIN))
#define LED0_OFF()  (LED0_PORT->ODR &= ~(1U << LED0_PIN))
#define LED0_TOG()  (LED0_PORT->ODR ^=  (1U << LED0_PIN))

// buzzer
#define BUZZER_PORT  GPIOC
#define BUZZER_PIN   9       /* PC9 */
#define BUZZER_ON()  (BUZZER_PORT->ODR |=  (1U << BUZZER_PIN))
#define BUZZER_OFF() (BUZZER_PORT->ODR &= ~(1U << BUZZER_PIN))
#define BUZZER_TOG() (BUZZER_PORT->ODR ^=  (1U << BUZZER_PIN))

// shift register pins
#define SR_DATA_PORT    GPIOB
#define SR_DATA_PIN     5    /* PB5  - serial data */
#define SR_CLK_PORT     GPIOA
#define SR_CLK_PIN      5    /* PA5  - shift clock */
#define SEG_LATCH_PORT  GPIOC
#define SEG_LATCH_PIN   10   /* PC10 - 7-seg latch */
#define LCD_LATCH_PORT  GPIOA
#define LCD_LATCH_PIN   10   /* PA10 - LCD latch */

 // keypad pins, PB1-4 column outputs, PB8-11 row inputs
#define KP_COL0_PIN  1    /* PB1 */
#define KP_COL1_PIN  2    /* PB2 */
#define KP_COL2_PIN  3    /* PB3 */
#define KP_COL3_PIN  4    /* PB4 */
#define KP_ROW0_PIN  8    /* PB8  */
#define KP_ROW1_PIN  9    /* PB9  */
#define KP_ROW2_PIN  10   /* PB10 */
#define KP_ROW3_PIN  11   /* PB11 */

// push buttons
#define SW4_PORT  GPIOC
#define SW4_PIN   13

#define SW5_PORT  GPIOD
#define SW5_PIN   3

// buffer sizes
#define DECODE_BUF_LEN  8
#define QUEUE_LEN       32

// FSM state enumerations
typedef enum {
    ENC_IDLE,
    ENC_LOAD_LETTER,
    ENC_START_SYMBOL,
    ENC_SOUND_ON,
    ENC_SOUND_OFF,
    ENC_INTER_LETTER,
    ENC_INTER_WORD,
    ENC_ERROR
} EncodeState;

typedef enum {
    DEC_IDLE,
    DEC_BUTTON_DOWN,
    DEC_CLASSIFY,
    DEC_APPEND_DOT,
    DEC_APPEND_DASH,
    DEC_WAIT_NEXT,
    DEC_DECODE_LETTER,
    DEC_WRITE_LCD,
    DEC_ERROR
} DecodeState;

typedef enum {
    MODE_ENCODE,
    MODE_DECODE
} SystemMode;

// global states
extern volatile SystemMode  g_mode;
extern volatile EncodeState g_enc_state;
extern volatile DecodeState g_dec_state;
extern volatile uint32_t    g_tim2_ticks;
extern volatile char        g_enc_pattern[8];
extern volatile int         g_enc_pattern_idx;
extern volatile int         g_enc_symbol_ticks;
extern volatile char        g_queue[QUEUE_LEN];
extern volatile int         g_queue_head;
extern volatile int         g_queue_tail;
extern volatile char        g_dec_buf[DECODE_BUF_LEN + 1];
extern volatile int         g_dec_buf_idx;
extern volatile uint8_t     g_flag_sw4_pressed;
extern volatile uint8_t     g_flag_sw4_released;
extern volatile uint8_t     g_flag_sw5_pressed;
extern volatile uint8_t     g_flag_tim6_fired;
extern volatile uint32_t    g_sw4_press_duration_ms;

// function prototypes

// gpio.c
void GPIO_Init_All(void);

// timers.c
void TIM2_Init(void);
void TIM3_Init(void);
void TIM6_Init(void);
void TIM2_Start(void);
void TIM2_Stop(void);
void TIM3_Reset_And_Start(void);
void TIM3_Stop(void);
uint32_t TIM3_Read_MS(void);
void TIM6_Start_OneShot(void);
void TIM6_Stop(void);

// exti.c
void EXTI_SW4_Init(void);
void EXTI_SW5_Init(void);

// lcd.c
void LCD_Init(void);
void LCD_Write_String(char *s);
void LCD_Write_Char(uint8_t c);
void LCD_Write_Instr(uint8_t instr);
void LCD_Clear(void);
void LCD_Set_Cursor(uint8_t row, uint8_t col);

// seg7.c
void Seg7_Init(void);
void Seg7_Display_Char(char c);
void Seg7_Clear(void);

// keypad.c
void Keypad_Init(void);
char Keypad_Read(void);         // returns confirmed letter, or 0
char Keypad_GetPreview(void);   // returns currently-selected multi-tap letter, or 0

// morse_table.c
const char* Morse_Encode(char c);
char        Morse_Decode(const char *pattern);

// encode.c
void Encode_Init(void);
void Encode_Run(void);
void Encode_Queue_Push(char c);

// decode.c
void Decode_Init(void);
void Decode_Run(void);

// system.c
void System_Clock_Config(void);
void Delay(uint32_t ms);
void Reset_All_State(void);
void Stop_All_Timers(void);
void Buzzer_Beep(uint32_t duration_ms);

#endif /* MORSE_H */
/*
 * file name: seg7.c
 *
 * displays the current encoded/decoded character on digit 2 (display 2).
 */

#include "morse.h"

/* 7-segment encoding for digits 0-9 and letters A-Z.
 * bit pattern: 0=segment ON (common anode, active LOW)
 * byte: {dp, g, f, e, d, c, b, a} with bit 7 always 1 to keep dot off.
 *
 * digit codes from lecture:
 *   0xC0=0, 0xF9=1, 0xA4=2, 0xB0=3, 0x99=4,
 *   0x92=5, 0x82=6, 0xF8=7, 0x80=8, 0x90=9
 */
static const uint8_t seg7_digits[10] = {
    0xC0, 0xF9, 0xA4, 0xB0, 0x99,
    0x92, 0x82, 0xF8, 0x80, 0x90
};

// rough letter encodings (best effort on 7-segment)
static const uint8_t seg7_letters[26] = {
    0x88, /* A */  0x83, /* B */  0xC6, /* C */  0xA1, /* D */
    0x86, /* E */  0x8E, /* F */  0xC2, /* G */  0x89, /* H */
    0xF9, /* I */  0xF1, /* J */  0x85, /* K */  0xC7, /* L */
    0xC8, /* M */  0xAB, /* N */  0xC0, /* O */  0x8C, /* P */
    0x98, /* Q */  0xAF, /* R */  0x92, /* S */  0x87, /* T */
    0xC1, /* U */  0xC1, /* V */  0x81, /* W */  0x89, /* X */
    0x91, /* Y */  0xA4, /* Z */
};

/*
 * Write_SR_7S
 * shift two bytes into the 7-segment shift register chain and latch.
 */
static void Write_SR_7S(uint8_t digit_data, uint8_t enable_data)
{
    uint8_t mask;

    mask = 0x80;
    for (int i = 0; i < 8; i++)
    {
        if (digit_data & mask)
            GPIOB->ODR |=  (1U << 5); 
        else
            GPIOB->ODR &= ~(1U << 5); 

        // pulse shift clock PA5
        GPIOA->ODR &= ~(1U << 5);
        GPIOA->ODR |=  (1U << 5);

        mask >>= 1;
    }

    mask = 0x80;
    for (int i = 0; i < 8; i++)
    {
        if (enable_data & mask)
            GPIOB->ODR |=  (1U << 5);
        else
            GPIOB->ODR &= ~(1U << 5);

        GPIOA->ODR &= ~(1U << 5);
        GPIOA->ODR |=  (1U << 5);

        mask >>= 1;
    }

    // pulse 7-seg latch PC10 to commit data to outputs 
    GPIOC->ODR |=  (1U << 10);
    GPIOC->ODR &= ~(1U << 10);
}

// enable arrays for displays 1-4 from lecture 
static const uint8_t enable[5] = {
    0x00,  
    0x08,  
    0x04,  
    0x02,  
    0x01,  
};

/*
 * Seg7_Init
 * just blank the display.
 */
void Seg7_Init(void)
{
    Write_SR_7S(0xFF, 0x00);   // all segments off, all digits disabled
}

/*
 * Seg7_Display_Char
 * display a single character on digit 2 of the 7-segment display.
 * clears the other three digits.
 */
void Seg7_Display_Char(char c)
{
    uint8_t seg;

    if (c >= '0' && c <= '9')
        seg = seg7_digits[c - '0'];
    else if (c >= 'A' && c <= 'Z')
        seg = seg7_letters[c - 'A'];
    else if (c >= 'a' && c <= 'z')
        seg = seg7_letters[c - 'a'];
    else
        seg = 0xFF;   // blank

    // display on digit 2
    Write_SR_7S(seg, enable[2]);
}

/*
 * Seg7_Clear
 * turn off all segments on all digits.
 */
void Seg7_Clear(void)
{
    Write_SR_7S(0xFF, 0x00);
}
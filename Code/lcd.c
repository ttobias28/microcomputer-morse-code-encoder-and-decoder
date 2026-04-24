/*
 * file name: lcd.c
 * 
 * LCD driver for the 2-line 16-character LCD 
 * uses a shift register interface (4-bit mode).
 */

#include "morse.h"

/*
Write_SR_LCD
shifts 8 bits into the LCD shift register, MSB first
after shifting, pulses the LCD latch (PA10) to commit the data
*/
static void Write_SR_LCD(uint8_t data)
{
    uint8_t mask = 0x80;  

    for (int i = 0; i < 8; i++)
    {
        // set data line (PB5) 
        if (data & mask)
            GPIOB->ODR |=  (1U << 5);
        else
            GPIOB->ODR &= ~(1U << 5);

        // pulse shift clock (PA5): LOW then HIGH
        GPIOA->ODR &= ~(1U << 5);
        GPIOA->ODR |=  (1U << 5);

        mask >>= 1;
    }

    // pulse latch clock (PA10) to load shift register into output
    GPIOA->ODR |=  (1U << 10);
    GPIOA->ODR &= ~(1U << 10);
}

/*
LCD_nibble_write
writes the upper 4 bits of 'data' to the LCD.
*/
static void LCD_nibble_write(uint8_t data, uint8_t mode)
{
    uint8_t out;

    if (mode == 0)  // command
    {
        out = (data & 0xF0) | 0x02;  
        Write_SR_LCD(out);
        out &= 0xFD;                  
        Write_SR_LCD(out);
    }
    else            // data
    {
        out = (data & 0xF0) | 0x03;
        Write_SR_LCD(out);
        out &= 0xFD;                  
        Write_SR_LCD(out);
    }
}


/*
LCD_Write_Instr
send an 8-bit command to the LCD
*/
void LCD_Write_Instr(uint8_t instr)
{
    LCD_nibble_write(instr & 0xF0, 0);          
    LCD_nibble_write((instr << 4) & 0xF0, 0);  
}

/*
LCD_Write_Char
send one ASCII character to the LCD at the current cursor position.
*/
void LCD_Write_Char(uint8_t c)
{
    LCD_nibble_write(c & 0xF0, 1);
    LCD_nibble_write((c << 4) & 0xF0, 1);
}

/*
LCD_Write_String
write a null-terminated string starting at the current cursor.
*/
void LCD_Write_String(char *s)
{
    while (*s)
        LCD_Write_Char((uint8_t)*s++);
}

/*
LCD_Clear
clear the display and return cursor to home.
*/
void LCD_Clear(void)
{
    LCD_Write_Instr(0x01);  
    Delay(5);               
}

/*
LCD_Set_Cursor
move cursor to (row, col).  row=0 is top line, row=1 is bottom line.
*/
void LCD_Set_Cursor(uint8_t row, uint8_t col)
{
    uint8_t addr;
    if (row == 0)
        addr = 0x80 | (col & 0x0F);  
    else
        addr = 0xC0 | (col & 0x0F); 
    LCD_Write_Instr(addr);
}

/*
LCD_Init
reset sequence and configuration
*/
void LCD_Init(void)
{
    // power-on reset sequence
    Delay(20);
    LCD_nibble_write(0x30, 0); Delay(5);
    LCD_nibble_write(0x30, 0); Delay(1);
    LCD_nibble_write(0x30, 0); Delay(1);
    LCD_nibble_write(0x20, 0); Delay(1);  

    // configure display
    LCD_Write_Instr(0x28); 
    LCD_Write_Instr(0x0E);  
    LCD_Write_Instr(0x01);  
    Delay(5);
    LCD_Write_Instr(0x06);  
}
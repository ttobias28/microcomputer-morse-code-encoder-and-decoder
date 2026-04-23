/*
 * file name: morse_table.c
 * 
 * bidirectional Morse code lookup table.
 *   Morse_Encode(char c)          ? returns ".-" string for that character
 *   Morse_Decode(const char *pat) ? returns the character for that pattern
 */

#include "morse.h"

/* =========================================================
 *  lookup table: character ? Morse pattern
 *  index 0 = 'A', 1 = 'B', ..., 25 = 'Z'
 *  numbers 0-9 are at indices 26-35
 *  NULL entry means "no Morse code defined"
 * ========================================================= */
static const char * const morse_table[] = {
    ".-",    /* A */
    "-...",  /* B */
    "-.-.",  /* C */
    "-..",   /* D */
    ".",     /* E */
    "..-.",  /* F */
    "--.",   /* G */
    "....",  /* H */
    "..",    /* I */
    ".---",  /* J */
    "-.-",   /* K */
    ".-..",  /* L */
    "--",    /* M */
    "-.",    /* N */
    "---",   /* O */
    ".--.",  /* P */
    "--.-",  /* Q */
    ".-.",   /* R */
    "...",   /* S */
    "-",     /* T */
    "..-",   /* U */
    "...-",  /* V */
    ".--",   /* W */
    "-..-",  /* X */
    "-.--",  /* Y */
    "--..",  /* Z */

    "-----", /* 0 */
    ".----", /* 1 */
    "..---", /* 2 */
    "...--", /* 3 */
    "....-", /* 4 */
    ".....", /* 5 */
    "-....", /* 6 */
    "--...", /* 7 */
    "---..", /* 8 */
    "----.", /* 9 */
};

#define TABLE_SIZE  (sizeof(morse_table) / sizeof(morse_table[0]))

/*
 * Morse_Encode
 * given an ASCII character, return its Morse pattern string.
 * returns NULL if the character has no Morse code.
 */
const char* Morse_Encode(char c)
{
    // convert lowercase to uppercase
    if (c >= 'a' && c <= 'z')
        c = c - 'a' + 'A';

    if (c >= 'A' && c <= 'Z')
        return morse_table[c - 'A'];        /* index 0..25 */

    if (c >= '0' && c <= '9')
        return morse_table[26 + (c - '0')]; /* index 26..35 */

    return NULL;  /* unknown character */
}

/*
 * Morse_Decode
 * given a dot/dash pattern string, return the matching character.
 * returns '?' if no match found.
 */
char Morse_Decode(const char *pattern)
{
    if (pattern == NULL || pattern[0] == '\0')
        return '?';

    // search letters A-Z (indices 0-25)
    for (int i = 0; i < 26; i++)
    {
        if (strcmp((const char*)pattern, morse_table[i]) == 0)
            return (char)('A' + i);
    }

    // search digits 0-9 (indices 26-35)
    for (int i = 0; i < 10; i++)
    {
        if (strcmp((const char*)pattern, morse_table[26 + i]) == 0)
            return (char)('0' + i);
    }

    return '?';   /* no match */
}
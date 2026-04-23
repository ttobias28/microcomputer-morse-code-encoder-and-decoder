/*
 * file name: keypad.c
 *
 * 4x4 matrix keypad driver with multi-tap (T9-style) letter input.
 *
 * key layout (EDUBase V2):
 *   1  2  3  A
 *   4  5  6  B
 *   7  8  9  C
 *   *  0  #  D
 *
 * multi-tap mapping — tap same key repeatedly to cycle:
 *   '1' -> E, F, G, 1    (tap 4 times to get the digit 1)
 *   '2' -> H, I, J, 2
 *   '3' -> K, L, M, 3
 *   '4' -> N, O, P, 4
 *   '5' -> Q, R, S, 5
 *   '6' -> T, U, V, 6
 *   '7' -> W, X, Y, Z, 7
 *   '8' -> 8 (direct — one tap)
 *   '9' -> 9 (direct — one tap)
 *   '0' -> 0 (direct — one tap)
 *   'A'-'D' -> letters A-D (direct)
 *   '*'  -> mode toggle (sets g_flag_sw5_pressed)
 *   '#'  -> word gap in encode mode
 *
 * to get digit 1: tap '1' four times (E -> F -> G -> 1 -> wraps to E)
 * to get digit 7: tap '7' five times (W -> X -> Y -> Z -> 7 -> wraps to W)
 *
 * Keypad_Read()       returns confirmed char or 0
 * Keypad_GetPreview() returns in-progress multi-tap letter/digit or 0
 */

#include "morse.h"

/* digit is included as the last entry in each group */
static const char mt1[] = {'E','F','G','1',0};
static const char mt2[] = {'H','I','J','2',0};
static const char mt3[] = {'K','L','M','3',0};
static const char mt4[] = {'N','O','P','4',0};
static const char mt5[] = {'Q','R','S','5',0};
static const char mt6[] = {'T','U','V','6',0};
static const char mt7[] = {'W','X','Y','Z','7',0};

static const uint8_t col_pins[4] = {1, 2, 3, 4};
static const uint8_t row_pins[4] = {8, 9, 10, 11};

static const char keymap[4][4] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

static char     mt_key   = 0;
static int      mt_count = 0;
static uint32_t mt_idle  = 0;

static const char* get_table(char k)
{
    switch(k){
        case '1': return mt1; case '2': return mt2; case '3': return mt3;
        case '4': return mt4; case '5': return mt5; case '6': return mt6;
        case '7': return mt7; default: return 0;
    }
}

static int table_len(char k)
{
    const char *t = get_table(k);
    if(!t) return 0;
    int n = 0; while(t[n]) n++; return n;
}

static char current_letter(void)
{
    if(!mt_key || !mt_count) return 0;
    const char *t = get_table(mt_key);
    if(!t) return 0;
    return t[(mt_count-1) % table_len(mt_key)];
}

static char confirm(void)
{
    char c = current_letter();
    mt_key = 0; mt_count = 0; mt_idle = 0;
    return c;
}

/*
 * scan_raw
 * returns the raw key pressed, or 0 if none.
 * IMPORTANT: only returns a key if at least one column is HIGH
 * when the row fires. this prevents SW2/PB8 phantom presses from
 * registering — SW2 pulls PB8 HIGH with no column driven.
 */
static char scan_raw(void)
{
    /* drive all columns HIGH */
    for(int c = 0; c < 4; c++) GPIOB->ODR |= (1U << col_pins[c]);

    /* quick check: any row HIGH? */
    if(((GPIOB->IDR >> 8) & 0xF) == 0) return 0;

    Delay(15);  /* debounce */
    if(((GPIOB->IDR >> 8) & 0xF) == 0) return 0;

    /* column-by-column scan */
    for(int c = 0; c < 4; c++){
        /* drive only this column HIGH, rest LOW */
        for(int cc = 0; cc < 4; cc++) GPIOB->ODR &= ~(1U << col_pins[cc]);
        GPIOB->ODR |= (1U << col_pins[c]);
        Delay(2);

        for(int r = 0; r < 4; r++){
            if(GPIOB->IDR & (1U << row_pins[r])){
                /* valid keypress: row HIGH with specific column driven */
                for(int cc = 0; cc < 4; cc++) GPIOB->ODR |= (1U << col_pins[cc]);
                return keymap[r][c];
            }
        }
    }

    /* no column/row match found — phantom press (e.g. SW2), ignore it */
    for(int c = 0; c < 4; c++) GPIOB->ODR |= (1U << col_pins[c]);
    return 0;
}

static void wait_release(void)
{
    for(int c = 0; c < 4; c++) GPIOB->ODR |= (1U << col_pins[c]);
    Delay(20);
    while((GPIOB->IDR >> 8) & 0xF){}
    Delay(20);
}

void Keypad_Init(void)
{
    uint32_t temp;
    /* columns PB1-PB4: push-pull output, no pull */
    for(int i = 0; i < 4; i++){
        uint8_t p = col_pins[i];
        temp = GPIOB->MODER; temp &= ~(0x3U<<(2*p)); temp |= (0x1U<<(2*p)); GPIOB->MODER = temp;
        GPIOB->OTYPER &= ~(1U << p);
        temp = GPIOB->PUPDR; temp &= ~(0x3U<<(2*p)); GPIOB->PUPDR = temp;
    }
    /* rows PB8-PB11: input, no pull (board has external pull-down) */
    for(int i = 0; i < 4; i++){
        uint8_t p = row_pins[i];
        temp = GPIOB->MODER; temp &= ~(0x3U<<(2*p)); GPIOB->MODER = temp;
        GPIOB->OTYPER &= ~(1U << p);
        temp = GPIOB->PUPDR; temp &= ~(0x3U<<(2*p)); GPIOB->PUPDR = temp;
    }
}

char Keypad_GetPreview(void)
{
    return current_letter();
}

char Keypad_Read(void)
{
    char raw = scan_raw();

    /* no key pressed */
    if(raw == 0){
        if(mt_key && mt_count && mt_key != '#'){
            mt_idle++;
            if(mt_idle >= MULTITAP_TIMEOUT_LOOPS)
                return confirm();
        }
        /* deferred '#' from previous press */
        if(mt_key == '#'){
            mt_key = 0; mt_count = 0; mt_idle = 0;
            return '#';
        }
        return 0;
    }

    wait_release();
    mt_idle = 0;

    /* '*': mode toggle — confirm any pending letter first */
    if(raw == '*'){
        char pending = 0;
        if(mt_key && mt_count && mt_key != '#')
            pending = confirm();
        g_flag_sw5_pressed = 1;
        return pending;
    }

    /* '#': word gap — confirm pending letter first */
    if(raw == '#'){
        if(mt_key && mt_count && mt_key != '#'){
            char letter = confirm();
            mt_key = '#'; mt_count = 1;
            return letter;
        }
        return '#';
    }

    /* direct keys: A B C D 0 8 9 */
    int is_direct = (raw=='A'||raw=='B'||raw=='C'||raw=='D'||
                     raw=='0'||raw=='8'||raw=='9');
    if(is_direct){
        char pending = 0;
        if(mt_key && mt_count && mt_key != '#') pending = confirm();
        if(pending){
            mt_key = raw; mt_count = 255; /* stash */
            return pending;
        }
        return raw;
    }

    /* stashed direct key */
    if(mt_key && mt_count == 255){
        char stashed = mt_key;
        mt_key = raw; mt_count = 1; mt_idle = 0;
        return stashed;
    }

    /* multi-tap keys 1-7 */
    if(raw >= '1' && raw <= '7'){
        if(raw == mt_key && mt_count != 255){
            /* same key: advance selection */
            mt_count++;
            if(mt_count > table_len(raw)) mt_count = 1;
            return 0;
        } else {
            /* different key: confirm previous, start new */
            char pending = 0;
            if(mt_key && mt_count && mt_count != 255) pending = confirm();
            mt_key = raw; mt_count = 1; mt_idle = 0;
            return pending;
        }
    }

    return 0;
}
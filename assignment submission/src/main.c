// libraries
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdio.h>
#include <avr/pgmspace.h>
// header files
#include "spi.h"
#include "timer.h"
#include "sequence.h"
#include "uart.h"
#include "buttons.h"

// arrays
static char ascii[4] = {0}; // store 4 sequence char
uint8_t decoded[3]; // store 3 bytes of decoded sequence
uint8_t descrambled[3]; // store 3 bytes of descrambled sequence

// variables
uint8_t sequence_index = 0; // hold sequence index
uint16_t offset = 0; // hold sequence offset
volatile uint8_t step = 0; // hold step
uint32_t state = 0x11050390; // hold state

//  calculating offset from sequence index
void sequnce_offset(uint8_t select)
{
    offset = select << 5;
}


// decoder
uint8_t base64(char ascii)
{
    if ('A' <= ascii && ascii <= 'Z')
    {
        ascii = ascii - 65;
        return ascii;
    }
    else if ('a' <= ascii && ascii <= 'z')
    {
        ascii = ascii - 97 + 26;
        return ascii;
    }
    else if ('0' <= ascii && ascii <= '9')
    {
        ascii = ascii + 4;
        return ascii;
    }
    else if ('+' == ascii)
    {
        ascii = ascii - 43 + 62;
        return ascii;
    }
    else if ('/' == ascii)
    {
        ascii = ascii - 47 + 63;
        return ascii;
    }
}

void decoder(char *ascii, uint8_t *decoded)
{
    // takes 4 characters from sequence
    uint8_t let1 = base64(ascii[0]);
    uint8_t let2 = base64(ascii[1]);
    uint8_t let3 = base64(ascii[2]);
    uint8_t let4 = base64(ascii[3]);

    // converts them to 3 bytes
    uint8_t byte1 = (let1 << 2) | (let2 >> 4);
    uint8_t byte2 = (let2 << 4) | (let3 >> 2);
    uint8_t byte3 = (let3 << 6) | let4;

    // stored decoded bytes in array
    decoded[0] = byte1;
    decoded[1] = byte2;
    decoded[2] = byte3;
}


void next(void){
    
 if ((state & 0x1) == 1)
 {
     state = state >> 1;
     state = state ^ 0xB4BCD35C;
 }
 else
 {
     state = state >> 1;
 }

}

// descrambler function
void descramble(uint8_t *a)
{
    for (uint8_t i = 0; i < 3; i++)
    {
        a = &decoded[i];
         uint32_t LSByte = (state & 0xFF);
        *a = *a ^ LSByte;
        descrambled[i] = *a;

        // advances state
        next();
    }
}



void advance(uint8_t sequence_index){
state = 0x11050390;
uint16_t advancement = 24 * sequence_index;
for (uint16_t i = 0; i < advancement; i++)
{
    next();
}
}

void adc_innit(void){
    ADC0.CTRLA = ADC_ENABLE_bm;
    ADC0.CTRLB = ADC_PRESC_DIV2_gc;
    ADC0.CTRLC = (4 << ADC_TIMEBASE_gp) | ADC_REFSEL_VDD_gc;
    ADC0.CTRLE = 64;
    ADC0.CTRLF = ADC_FREERUN_bm | ADC_LEFTADJ_bm;
    ADC0.MUXPOS = ADC_MUXPOS_AIN2_gc;
    ADC0.COMMAND = ADC_MODE_SINGLE_8BIT_gc | ADC_START_IMMEDIATE_gc;
}

// change to 10Mhz clock
void clock_init (void){
    CCP = CCP_IOREG_gc;
    CLKCTRL.MCLKCTRLB = CLKCTRL_PEN_bm;
}

void pwm_innit(void)
{
    PORTB_DIRSET = PIN1_bm;
    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV4_gc;
    TCA0.SINGLE.CTRLB = TCA_SINGLE_CMP0_bm | TCA_SINGLE_CMP1EN_bm |      TCA_SINGLE_WGMODE_SINGLESLOPE_gc;
    TCA0.SINGLE.PER = 30303; // 3333333/110
    TCA0.SINGLE.CMP0 = 15151; // 50% duty cycle
    TCA0.SINGLE.CMP1 = 30303; // 3333333/110 * 0.15 (15% duty cycle)
    TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;
}

// enum for sequence select
typedef enum{
    SEQUNCE_SELECT,
    SEQUENCING,
    TEST
} mode;
// starting case
mode modes = SEQUNCE_SELECT;

// enum for buttons
typedef enum{
    START,
    INC,
    DEC,
    PETENTIOMETER,
    PLAY
} button;
// starting case
button buttons = START;

uint32_t PerFrequencyTable[16][12] = {
{           0,           0,           0,           0,           0,           0,           0,           0,           0,           0,           0,           0},
{       45455,       42903,       40495,       38223,       36077,       34052,       32141,       30337,       28635,       27027,       25511,       24079},
{       22727,       21452,       20248,       19111,       18039,       17026,       16071,       15169,       14317,       13514,       12755,       12039},
{       11364,       10726,       10124,        9556,        9019,        8513,        8035,        7584,        7159,        6757,        6378,        6020},
{        5682,        5363,        5062,        4778,        4510,        4257,        4018,        3792,        3579,        3378,        3189,        3010},
{        2841,        2681,        2531,        2389,        2255,        2128,        2009,        1896,        1790,        1689,        1594,        1505},
{        1420,        1341,        1265,        1194,        1127,        1064,        1004,         948,         895,         845,         797,         752},
{         710,         670,         633,         597,         564,         532,         502,         474,         447,         422,         399,         376},
{         355,         335,         316,         299,         282,         266,         251,         237,         224,         211,         199,         188},
{         178,         168,         158,         149,         141,         133,         126,         119,         112,         106,         100,          94},
{          89,          84,          79,          75,          70,          67,          63,          59,          56,          53,          50,          47},
{          44,          42,          40,          37,          35,          33,          31,          30,          28,          26,          25,          24},
{          22,          21,          20,          19,          18,          17,          16,          15,          14,          13,          12,          12},
{          11,          10,          10,           9,           9,           8,           8,           7,           7,           7,           6,           6},
{           6,           5,           5,           5,           4,           4,           4,           4,           3,           3,           3,           3},
{           3,           3,           2,           2,           2,           2,           2,           2,           2,           2,           2,           1}
};


void get_sequence(void){
    // get four characters from sequence array 
    // put those 4 char in the ascii array 
    // to convert base 64 to ascii
    offset = sequence_index << 5;
    memcpy_P(ascii, &SEQUENCE[ + offset + step], 4);
    step+=4;
}

void execute_step(void){
if (descrambled[0] == 0)
{
    modes = SEQUNCE_SELECT;
    
} else{
    uint8_t duration = descrambled[0];
    uint8_t octave = ((descrambled[2] & 0xF0)>>4);
    uint8_t note = (descrambled[2] & 0x0F);
    uint32_t frequency = PerFrequencyTable[octave][note];
    uint32_t brightness = descrambled[1];
    count = 0;
    while (count <= duration)
{
    // set buzzer frequency
    PORTB.DIRSET = PIN0_bm;
    TCA0.SINGLE.PERBUF = frequency;
    // set 50% duty cyle
    TCA0.SINGLE.CMP0BUF = (frequency)>>1; // bitshift ins divide by 2
    // set brightness
    TCA0.SINGLE.CMP1BUF = (((uint32_t)brightness) * (uint16_t)TCA0.SINGLE.PER) >> 8;

}}}

int main(void)
{
    // initialisations
    cli();
    uart_init();
    spi_init();
    timer_init();
    pwm_innit();
    buttons_init();
    adc_innit();
    clock_init();
    sei();

    uint8_t pb_old = 0xFF;
    PORTB.DIRCLR = PIN0_bm; // turn buzzer off
    PORTA.DIRSET = PIN1_bm;
    PORTB.OUTSET = PIN5_bm;
    PORTB.DIRSET = PIN5_bm;
    display_hexx(0);
    while(1){
        
        switch (buttons)
        {
        case SEQUNCE_SELECT:;
            uint8_t pressed = (pb_state ^ pb_old) & (pb_old);
            pb_old = pb_state;
            if (pressed & PIN6_bm && sequence_index != 0xFF){
                sequence_index++;
                display_hexx(sequence_index);
            }
            if (pressed & PIN5_bm &&  sequence_index != 0x00){
                sequence_index--;
                display_hexx(sequence_index);
            }
            if(PIN4_bm & ~pb_state){
                uint32_t t = ADC0.RESULT;
                sequence_index = t;
                display_hexx(sequence_index);
            }
            if (pressed & PIN7_bm){
                modes == SEQUENCING;
            }
            break;
        case SEQUENCING:
        advance(sequence_index);
        step = 0;
            LHS = 0b10000000;
            RHS = 0b10000000;
            get_sequence();
            decoder(ascii, decoded);
            uint8_t *b;
            descramble(b);
            execute_step();
    
        
            break;
        
        default:
            break;
        }
    }


} // end main()

#include <avr/io.h>
#include <avr/interrupt.h>
#include "spi.h"

volatile uint8_t LHS;
volatile uint8_t RHS;
volatile uint8_t count;
volatile uint8_t measure_count;
volatile uint8_t pb_state = 0xFF;
uint8_t pb_falling_edge = 0x00;
uint8_t pressed;

void timer_init() {
    TCB0.CTRLB = TCB_CNTMODE_INT_gc;    // Configure TCB0 in periodic interrupt mode
    TCB0.CCMP = 10000;                   // Set interval for 1ms (3333 clocks @ 3.3 MHz) 10 000 @ 10 MHz
    TCB0.INTCTRL = TCB_CAPT_bm;         // CAPT interrupt enable
    TCB0.CTRLA = TCB_ENABLE_bm;         // Enable

    
    TCB1.CTRLB = TCB_CNTMODE_INT_gc;    // Configure TCB0 in periodic interrupt mode
    TCB1.CCMP = 65535;                   // Set interval for 13.1ms (3333 clocks @ 3.3 MHz
    TCB1.INTCTRL = TCB_CAPT_bm;         // CAPT interrupt enable
    TCB1.CTRLA = TCB_ENABLE_bm;       // Enable
}

const uint8_t display[16] = {
    0b00001000, // 0
    0b01101011, // 1
    0b01000100, // 2
    0b01000001, // 3
    0b00100011, // 4
    0b00010001, // 5
    0b00010000, // 6
    0b01001011, // 7
    0b00000000, // 8
    0b00000011, // 9
    0b00000010, // A
    0b00110000, // b
    0b00011100, // C
    0b01100000, // d
    0b00010100, // E
    0b00010110, // F
};

void display_hexx(uint8_t sequence_index){
    RHS = (display[(sequence_index & 0x0F)]);
    LHS = (display[(sequence_index >> 4)])^0x80;
}


ISR(TCB0_INT_vect) {

    // display flickering
    static uint8_t side = 0;
    if (side)
    {
        spi_write(RHS);   // LHS

    }else{
        spi_write(LHS); //RHS
    }
    side ^= 1;


    //static uint8_t pb_previous = 0xFF;
    pb_state = PORTA.IN;
    uint8_t count0 = 0;
    uint8_t count1 = 0;
    uint8_t pbsample = PORTA.IN;
    uint8_t pbchanged = pbsample ^ pb_state;
    count1 = (count1 ^ count0) & pbchanged;
    count0 = ~count0  & pbchanged;
    pb_state ^= (count1 & count0) | (pbchanged & pb_state);

    measure_count++;
    if (measure_count == 13)
    {
        count++;
        measure_count = 0;
    }
    TCB0.INTFLAGS = TCB_CAPT_bm;        // Acknowledge interrupt
    
}

ISR(TCB1_INT_vect){
    count++;
    TCB1.INTFLAGS = TCB_CAPT_bm;
}

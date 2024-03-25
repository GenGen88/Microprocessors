#include <avr/io.h>
#include <avr/interrupt.h>

//volatile uint8_t pb_state = 0xFF;

void buttons_init() {
    // Enable pull-up resistors for PBs
    PORTA.PIN4CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN5CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN6CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN7CTRL = PORT_PULLUPEN_bm;

/*
    // Configure timer for PB sampling
    cli();
    TCB0.CTRLB = TCB_CNTMODE_INT_gc;    // Configure TCB0 in periodic interrupt mode
    TCB0.CCMP = 3333;                   // Set interval for 1ms (3333 clocks @ 3.3 MHz)
    TCB0.INTCTRL = TCB_CAPT_bm;         // CAPT interrupt enable
    TCB0.CTRLA = TCB_ENABLE_bm;         // Enable
    sei(); 
*/
}
/*
// Timer ISR; samples pushbuttons
ISR(TCB0_INT_vect) {
    static uint8_t pb_previous = 0xFF;
    pb_state = PORTA.IN;
    uint8_t count0 = 0;
    uint8_t count1 = 0;
    uint8_t pbsample = PORTA.IN;
    uint8_t pbchanged = pbsample ^ pb_state;
    count1 = (count1 ^ count0) & pbchanged;
    count0 = ~count0  & pbchanged;
    pb_state ^= (count1 & count0) | (pbchanged & pb_state);
       
    TCB0.INTFLAGS = TCB_CAPT_bm;        // Acknowledge interrupt
}
*/
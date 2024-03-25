#include <avr/io.h>
#include <avr/interrupt.h>

void timer_init();
extern volatile uint8_t LHS;
extern volatile uint8_t RHS;
extern volatile uint8_t pb_state;
extern volatile uint8_t count;
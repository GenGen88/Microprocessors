#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

ISR(SPI0_INT_vect){
    PORTA.OUTSET = PIN1_bm;
    PORTA.OUTCLR = PIN1_bm; // to make rising edge every time
}

void spi_init(void){ // intialise SPI so it can be used to control LED
    PORTMUX.SPIROUTEA |= PORTMUX_SPI0_ALT1_gc;  // pick SPI controls, then make it use port C ins default port A
    SPI0.CTRLA |= SPI_MASTER_bm; // put it into Master out slave in mode?? (host operation mode)
    SPI0.CTRLB |= SPI_SSD_bm;  // disable client select mode (no buffer mode)
    SPI0.INTCTRL |= SPI_IE_bm; // enables SPI0 interrupt via ie bit
    PORTA.DIRSET = PIN1_bm; // set state of DISPATCH pin to output to control LED
    PORTC.DIRSET = PIN0_bm | PIN2_bm; // set state of SPI MOSI and SPI CLK to high
    PORTB.OUTSET = PIN1_bm; // set output value of DISP EN to high
    PORTB.DIRCLR = PIN5_bm; // set output value of DISP DP to low to turn it off??
    SPI0.CTRLA |= SPI_ENABLE_bm; // enable at end
}

void spi_write(uint8_t b){
    SPI0.DATA = b;
}
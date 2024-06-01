#include <msp430.h>



void main(void) {
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer

    init();

    __bis_SR_register(GIE); // Interrupt enable

    while (1)
    {

    }
}

void init(void){

}
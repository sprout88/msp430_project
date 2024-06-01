#include <msp430.h>

unsigned int is_left_switch = 0;
unsigned int is_right_switch = 0;

void main(void) {
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer

    // Left Switch
    P2OUT |= BIT1;
    P2REN |= BIT1;

    // Right Switch
    P1OUT |= BIT1;
    P1REN |= BIT1;

    while(1)
    {

        if((P2IN & BIT1) == 0) // Left Switch pushed
        {
            is_left_switch = 1;
        }
        if((P1IN & BIT1) == 0) // Right Switch pushed
        {
            is_right_switch = 1;
        }
    }

}

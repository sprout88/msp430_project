#include <msp430.h>

unsigned int data = 0;

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;

    P1DIR |= BIT0;
    P4DIR |= BIT7;

    
}
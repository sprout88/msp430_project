#include <msp430.h>

unsigned int data = 0;

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;

    /* LED1 : P1-0 */
    P1DIR |= BIT0;
    /* LED1 END */

    /* LED2 : P4-7 */
    P4DIR |= BIT7;
    /* LED2 END */

    while(1){
        P1OUT |= BIT0; // LED1 ON
        P4OUT |= BIT7; // LED2 ON

        __delay_cycles(500000);

        P1OUT &= ~BIT0; // LED1 OFF
        P4OUT &= ~BIT7; // LED2 OFF

        __delay_cycles(500000);
    }
}

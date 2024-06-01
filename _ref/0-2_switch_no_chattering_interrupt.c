#include <msp430f5529.h>

unsigned int is_left_switch = 0;
unsigned int is_right_switch = 0;

void main(void) {
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer

    /* Left Switch */
    // Left Switch 
    P2OUT |= BIT1; // DIR
    P2REN |= BIT1; // pull up resister

    // Left Switch's Interrupt
    P2IE |= BIT1; // Interrupt Enable
    P2IES |= BIT1; // Interrupt edge select : Falling Edge
    P2IFG &= ~BIT1; // interrupt flag
    /* END Left Switch */

    /* Left Switch */
    // Right Switch
    P1OUT |= BIT1; // DIR
    P1REN |= BIT1; // pull up resister

    // Right Switch's Interrupt
    P1IE |= BIT1; // Interrupt Enable
    P1IES |= BIT1; // Interrupt edge select : Falling Edge
    P1IFG &= ~BIT1; // interrupt flag
    /* END Right Switch */

    __bis_SR_register(GIE); // Interrupt enable

    while(1)
    {

    }
}
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
    if((P2IN & BIT1) == 0)
    {
        is_left_switch = 1;
    }
    P2IFG &= ~BIT1; // IFG clear (Interrupt END)
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    if((P1IN & BIT1) == 0)
    {
        is_right_switch = 1;
    }
    P1IFG &= ~BIT1; // IFG clear (Interrupt END)
}

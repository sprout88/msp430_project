#include <msp430.h>

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    // PWN set
    P2DIR |= (BIT5 | BIT4);
    P2SEL |= (BIT5 | BIT4);
    TA2CCR0 = 1000;
    TA2CCTL2 = OUTMOD_6;
    TA2CCR2 = 0;
    TA2CCTL1 = OUTMOD_6;
    TA2CCR1 = 0;
    TA2CTL = TASSEL_2 + MC_1;

    __bis_SR_register(GIE);

    while(1)
    {
        TA2CCR2 = 0;
        TA2CCR1 = 500;
    }
}

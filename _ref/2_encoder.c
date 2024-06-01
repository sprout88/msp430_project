#include <msp430.h>

unsinged int digits[10] = { 0xdb, 0x50, 0x1f, 0x5d, 0xd4, 0xcd, 0xcf, 0xd8, 0xdf, 0xdd };

int encoder_cnt = 0;

unsinged int cnt = 0;

void main(void){
    WDTCTL = WDTPW | WDTHOLD;


    // digital segment
    P3DIR |= 0xffff;
    P3OUT &= 0x0000;
    P4DIR |= 0x000f;
    P4OUT &= ~BIT0;

    // interrupt set
    P1IE |= BIT5; // interrupt1 enabled
    P1IES |= BIT5 // interrupt1 : falling edge
    P1IFG &= ~BIT5; // interrupt1 flag

    P1IE |= BIT4; // interrupt2 enabled
    P1IES |= BIT4; // interrupt2 : falling edge
    P1IFG &= ~BIT4; // interrupt2 flag

    __bis_SR_register(GIE);

    while(1)
    {
        P3OUT = digits[encoder_cnt%10]; // 0001
        P4OUT = ~BIT0; // P4 : OFF OFF OFF ON
        __delay_cycles(5000);

        P3OUT = digits[encoder_cnt/10%10]; // 0010
        P4OUT = ~BIT1; // P4 : OFF OFF ON OFF
        __delay_cycles(5000);

        P3OUT = digits[encoder_cnt/100%10]; // 0100
        P4OUT = ~BIT2; // P4 : OFF ON OFF OFF
        __delay_cycles(5000);

        P3OUT = digits[encoder_cnt/1000%10]; // 1000
        P4OUT = ~BIT3; // P4 : ON OFF OFF OFF
        __delay_cycles(5000);

    }
}
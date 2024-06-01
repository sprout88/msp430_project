#include <msp430.h>

unsigned int digits[10] = { 0xdb, 0x50, 0x1f, 0x5d, 0xd4, 0xcd, 0xcf, 0xd8, 0xdf, 0xdd };
int encoder_cnt = 0;
unsigned int dynamic_segment_cnt = 0;

void main(void) {
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer

    // Digital Output - 7 segment
    P3DIR |= 0xffff;
    P3OUT &= 0x0000;
    P4DIR |= 0x000f;
    P4OUT &= ~BIT0;

    // Encoder
    // EncoderA and EncoderB -> 위상 엇갈림
    // EncoderA : P1_2
    // EncoderB : P1_3    

    P1IE |= BIT2; // Interrupt enabled
    P1IES |= BIT2; // Interrupt edge (Falling Edge)
    P1IFG &= ~BIT2; // Interrupt flag

    P1IE |= BIT3; // Interrupt enabled
    P1IES |= BIT3; // Interrupt edge (Falling Edge)
    P1IFG &= ~BIT3; // Interrupt flag

    __bis_SR_register(GIE); // Enable global interrupts

    while(1) {
        P3OUT = digits[encoder_cnt % 10];
        P4OUT = ~BIT0; // P4: OFF OFF OFF ON
        __delay_cycles(5000);

        P3OUT = digits[(encoder_cnt / 10) % 10];
        P4OUT = ~BIT1; // P4: OFF OFF ON OFF
        __delay_cycles(5000);

        P3OUT = digits[(encoder_cnt / 100) % 10];
        P4OUT = ~BIT2; // P4: OFF ON OFF OFF
        __delay_cycles(5000);

        P3OUT = digits[(encoder_cnt / 1000) % 10];
        P4OUT = ~BIT3; // P4: ON OFF OFF OFF
        __delay_cycles(5000);
    }
}
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void) {
    if (P1IFG & BIT3) { // encoder interrupt
        if ((P1IN & BIT2) != 0) {
            encoder_cnt--;
        } else {
            encoder_cnt++;
        }
    }

    if (P1IFG & BIT2) { // encoder interrupt
        if ((P1IN & BIT3) == 0) {
            encoder_cnt--;
        } else {
            encoder_cnt++;
        }
    }

    if (encoder_cnt > 9999) {
        encoder_cnt = 0;
    } else if (encoder_cnt < 0) {
        encoder_cnt = 9999;
    }

    P1IFG &= ~BIT3; // Clear IFG for BIT3
    P1IFG &= ~BIT2; // Clear IFG for BIT2
}

#include <msp430.h>

unsigned int digits[] = { 0xdb, 0x50, 0x1f, 0x5d, 0xd4, 0xcd, 0xcf, 0xd8, 0xdf, 0xdd };

int encoder_cnt = 0;

unsigned int dynamic_segment_cnt = 0;

void main(void){
    WDTCTL = WDTPW | WDTHOLD;
    
    // digital 7 segment
    P3DIR |= 0xffff;
    P3OUT &= 0x0000;
    P4DIR |= 0x000f;
    P4OUT &= ~BIT0;

    __bis_SR_register(GIE);

    while(1)
    {
        P3OUT = 0xdb;
    }
}

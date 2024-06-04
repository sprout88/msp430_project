#include <msp430.h>

unsigned int digits[] = { 0xdb, 0x50, 0x1f, 0x5d, 0xd4, 0xcd, 0xcf, 0xd8, 0xdf, 0xdd };

int encoder_cnt = 0;

unsigned int dynamic_segment_cnt = 0;

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    // digital 7 segment
    P3DIR |= 0xffff; // segment LED dir
    P3OUT &= 0x0000; // segment LED 모두 끄기
    P4DIR |= 0x000f; // segment 4LED gnd dir

    P3OUT = 0xdb; // 모든 세그먼트에 숫자 0 출력

    /* HEX 의 하위 4자리만 영향 받음. */

    P4OUT = 0x00; // OOOO
    P4OUT = 0x01; // OOOX
    P4OUT = 0x02; // OOXO
    P4OUT = 0x03; // OOXO
    P4OUT = 0x04; // OXOO
    P4OUT = 0x05; // OXOX
    P4OUT = 0x06; // OXXO
    P4OUT = 0x07; // OXXX
    P4OUT = 0x08; // XOOO
    P4OUT = 0x09; // XOOX
    P4OUT = 0x0A; // XOXO
    P4OUT = 0x0B; // XOXX
    P4OUT = 0x0C; // XXOO
    P4OUT = 0x0D; // XXOX
    P4OUT = 0x0E; // XXXO
    P4OUT = 0x0F; // XXXX
    P4OUT = 0x10; // OOOO
    P4OUT = 0x11; // OOO1

    __bis_SR_register(GIE);

    while(1)
    {

    }
}

#include <msp430.h>

int encoder_cnt = 0;
int save = 0;

void main(void) {
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer

    // Right Switch
    P1OUT |= BIT1;
    P1REN |= BIT1;

    // Right Switch's Interrupt
    P1IE |= BIT1; // Interrupt Enable
    P1IES |= BIT1; // Interrupt edge select : Falling Edge
    P1IFG &= ~BIT1; // interrupt flag

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

    __bis_SR_register(GIE); // Interrupt enable
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    // 엔코더를 돌리면, encoder_cnt 값이 변화
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

    // 엔코더 오버플로우 핸들링
    if (encoder_cnt > 9999) {
        encoder_cnt = 0;
    } else if (encoder_cnt < 0) {
        encoder_cnt = 9999;
    }

    // 스위치 누르면 encoder_cnt 값 저장
    if((P1IN & BIT1) == 0)
    {
        is_right_switch = 1;
    }

    P1IFG &= ~BIT3; // IFG clear (Interrupt END)
    P1IFG &= ~BIT1; // IFG clear (Interrupt END)
}

#include <msp430.h>

int encoder_cnt = 0;
int password = 0;
int key = 0; // keypad 에서 입력한 값을 저장하는 임시변수.
int segment_place = 0; // 4자리 segment 의 각 자리의 현재 선택을 의미, hardware: |0 1 2 3|
int data[4] = {0,}; // data: |0,0,0,0|, data[십진자릿수]
//pwm_data = data[3] * 1000 + data[2] * 100 + data[1] * 10 + data[0];
unsigned int i = 0;

void init(void);
void keypad_controller(void);

void main(void) {
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer

    init();

    __bis_SR_register(GIE); // Interrupt enable

    while (1)
    {
        keypad_controller();
    }
}

void init(void){
    // Right Switch
    P1OUT |= BIT1;
    P1REN |= BIT1;

    // Right Switch's Interrupt
    P1IE |= BIT1; // Interrupt Enable
    P1IES |= BIT1; // Interrupt edge select : Falling Edge
    P1IFG &= ~BIT1; // interrupt flag

    /* Encoder */ 

    // EncoderA and EncoderB -> 위상 엇갈림
    // EncoderA : P1_2
    // EncoderB : P1_3    

    P1IE |= BIT2; // Interrupt enabled
    P1IES |= BIT2; // Interrupt edge (Falling Edge)
    P1IFG &= ~BIT2; // Interrupt flag

    P1IE |= BIT3; // Interrupt enabled
    P1IES |= BIT3; // Interrupt edge (Falling Edge)
    P1IFG &= ~BIT3; // Interrupt flag
    /* END Encoder */ 
}

void keypad_controller(void){
    /*Keypad Controller*/

    // columns 1
    P2OUT &= ~BIT2;
    P2OUT |= (BIT0 | BIT3);

    if ((P6IN & BIT3) == 0) // Button 1
    {
        key = 1;
    }
    else if ((P6IN & BIT6) == 0) // Button 4
    {
        key = 4;
    }
    else if ((P6IN & BIT5) == 0) // Button 7
    {
        key = 7;
    }
    else if ((P6IN & BIT4) == 0) // Button *
    {
        while ((P6IN & BIT4) == 0) // * 을 누르면, 다음 segment_place 로 이동
            key = 0; // segment_place 이동 전 key 초기화
        segment_place++; // segment_place 이동
    }

    // columns 2
    P2OUT &= ~BIT0;
    P2OUT |= (BIT2 | BIT3);

    if ((P6IN & BIT3) == 0) // Button 2
    {
        key = 2;
    }
    else if ((P6IN & BIT6) == 0) // Button 5
    {
        key = 5;
    }
    else if ((P6IN & BIT5) == 0) // Button 8
    {
        key = 8;
    }
    else if ((P6IN & BIT4) == 0) // Button 0
    {
        key = 0;
    }

    // columns 3
    P2OUT &= ~BIT3;
    P2OUT |= (BIT0 | BIT2);

    if ((P6IN & BIT3) == 0) // Button 3
    {
        key = 3;
    }
    else if ((P6IN & BIT6) == 0) // Button 6
    {
        key = 6;
    }
    else if ((P6IN & BIT5) == 0) // Button 9
    {
        key = 9;
    }
    else if ((P6IN & BIT4) == 0) // Button # 을 누르면 전체 password data 초기화
    {
        segment_place = 0; // segment_place 초기화

        for (i = 0; i < 4; i++) // data 초기화
        {
            data[i] = 0;
        }
    }

    // data[segment_place] 에 key 를 저장.
    if (segment_place == 0)
    {
        data[3] = key;
    }
    else if (segment_place == 1)
    {
        data[2] = key;
    }
    else if (segment_place == 2)
    {
        data[1] = key;
    }
    else if (segment_place == 3)
    {
        data[0] = key;
    }
    else if (segment_place == 4) // password 입력 완료
    {
        
    }
    /* END Keypad Controller*/
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
        password = encoder_cnt;
    }

    P1IFG &= ~BIT3; // Encoder A(P1_3) IFG clear (Interrupt END)
    P1IFG &= ~BIT2; // Encoder B(P1_2) IFG clear (Interrupt END)
    P1IFG &= ~BIT1; // Right Switch(P1) IFG clear (Interrupt END)
}

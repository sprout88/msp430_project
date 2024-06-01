#include <msp430.h>

unsigned int encoder_cnt = 0;
unsigned int password = 0;
unsigned int key = 0; // keypad 에서 입력한 값을 저장하는 임시변수.
unsigned int place = 0; // 4자리 segment 의 각 자리의 현재 선택을 의미, hardware: |0 1 2 3|
unsigned int data[4] = {0,}; // data: |0,0,0,0|, data[십진자릿수]
//pwm_data = data[3] * 1000 + data[2] * 100 + data[1] * 10 + data[0];
unsigned int i = 0;
unsigned int dynamic_segment_cnt = 0;
unsigned int data_value = 0; // data_value = data[3] * 1000 + data[2] * 100 + data[1] * 10 + data[0];

unsigned int digits[10] = { 0xdb, 0x50, 0x1f, 0x5d, 0xd4, 0xcd, 0xcf, 0xd8, 0xdf, 0xdd}; // 7 segment digits

void init(void);
void keypad_controller(void);
void show_screen(unsigned int);
void reset_password_input(void);
void stanby(void);

void main(void) {
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer

    init();

    __bis_SR_register(GIE); // Interrupt enable

    while (1)
    {
        data_value = data[3] * 1000 + data[2] * 100 + data[1] * 10 + data[0];
        keypad_controller();
        //show_screen(data_value);
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

    /* 7 segment Digital Output */
    P3DIR |= 0xffff;
    P3OUT &= 0x0000;
    P4DIR |= 0x000f;
    P4OUT &= ~BIT0;
    /* END 7 segment Digital Output */

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

    /* keypad */
    // output
    P2DIR |= (BIT0 | BIT2 | BIT3);
    P2OUT |= (BIT0 | BIT2 | BIT3); // all high

    // input
    P6REN |= (BIT3 | BIT4 | BIT5 | BIT6);
    P6OUT |= (BIT3 | BIT4 | BIT5 | BIT6); // pull up
    /* END keypad */

    /* Timer - Timer0 */
    TA0CCTL0 = CCIE;
    TA0CCR0 = 5000; //1000;
    TA0CTL = TASSEL_2 + MC_1 + TACLR; // SMCLK : 1Mhz / Up mode to CCRO
    /* END Timer - Timer0 */
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
        while ((P6IN & BIT4) == 0) // * 을 누르면, 다음 place 로 이동
            key = 0; // place 이동 전 key 초기화
        place++; // place 이동
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
        reset_password_input();
    }

    // data[place] 에 key 를 저장.
    if (place == 0)
    {
        data[3] = key;
    }
    else if (place == 1)
    {
        data[2] = key;
    }
    else if (place == 2)
    {
        data[1] = key;
    }
    else if (place == 3)
    {
        data[0] = key;
    }
    else if (place == 4) // password 입력 완료
    {
        if(data_value == password)
        {
            stanby();
        }else{
            reset_password_input();
        }
    }
    /* END Keypad Controller*/
}

void show_screen(unsigned int value){

    if (dynamic_segment_cnt > 3)
        dynamic_segment_cnt = 0; // count 순회

    switch (dynamic_segment_cnt)
    {
    case 0:
        P3OUT = digits[value%10];
        P4OUT = ~BIT0;
        break;
    case 1:
        P3OUT = digits[value/10%10];
        P4OUT = ~BIT1;
        break;
    case 2:
        P3OUT = digits[value/100%10];
        P4OUT = ~BIT2;
        break;
    case 3:
        P3OUT = digits[value/1000%10];
        P4OUT = ~BIT3;
        break;
    }
}

void reset_password_input(void){
    place = 0; // place 초기화

    for (i = 0; i < 4; i++) // data 초기화
    {
        data[i] = 0;
    }
}

void stanby(void){
    
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

//    // password 값을 스크린에 표시
//    show_screen(encoder_cnt);

    P1IFG &= ~BIT3; // Encoder A(P1_3) IFG clear (Interrupt END)
    P1IFG &= ~BIT2; // Encoder B(P1_2) IFG clear (Interrupt END)
    P1IFG &= ~BIT1; // Right Switch(P1) IFG clear (Interrupt END)
}

// Timer interrupt service routine
// 7 Segment Dinamic 구동 타이머
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
    dynamic_segment_cnt++;
    show_screen(encoder_cnt);
}

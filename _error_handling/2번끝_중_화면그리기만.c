#include <msp430.h>
unsigned int a = 0;
unsigned int encoder_count = 0;
unsigned cnt = 0;
unsigned data = 0;        // ADC 값 받아오는 데이터 값
unsigned menu = 0;               // P1.1 스위치를 누름에 따라 증가하는 값
unsigned int adc_data = 0; // ADC 값 매핑
unsigned int adc_value[4] = {0, }; //세그먼트에 넣어주게 만듦
unsigned state = 0;               // 전체적인 동작의 상태 설정
unsigned pw[4] = {0, };          // encoder_count를 이용한 비밀번호 설정에 이용되는 변수
unsigned key = 0;
unsigned keymenu = 0;            // 키패드에서 *을 누를때마다 증가하는 변수
unsigned i = 0;
unsigned int led_cnt = 0; // led 타이머에 쓸 카운트
unsigned int led_cnt_req = 0; // adc값에 따라 켜지는 시간을 정하는 변수
unsigned int cho_stop = 0; //초음파 센서에 사용하는 변수
unsigned int cho_data = 0;
unsigned long cho_timer = 0;
unsigned int motor_for = 0; // 모터 속도를 저장할 변수 추가
unsigned int motor_rev = 0; // 모터 속도를 저장할 변수 추가
unsigned int pre_motor_for = 0; // 이전 모터 속도 변수
unsigned int pre_motor_rev = 0; // 이전 모터 속도 변수

int seg[10] ={
              BIT0 | BIT1 | BIT3 | BIT4 | BIT6 | BIT7,    // P3.0, 3.1, 3.3, 3.4, 3.6, 3.7이 1이 되면 0을 출력
              BIT4 | BIT6,                                // 1
              BIT0 | BIT1 | BIT2 | BIT3 | BIT4,           // 2
              BIT0 | BIT2 | BIT3 | BIT4 | BIT6,           // 3
              BIT2 | BIT4 | BIT6 | BIT7,                  // 4
              BIT0 | BIT2 | BIT3 | BIT6 | BIT7,           // 5
              BIT0 | BIT1 | BIT2 | BIT3 | BIT6 | BIT7,    // 6
              BIT3 | BIT4 | BIT6,                         // 7
              BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT6 | BIT7, // 8
              BIT0 | BIT2 | BIT3 | BIT4 | BIT6 | BIT7     // 9
              };

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    // Digital Output - 7 segment
    P3DIR |= 0xffff;
    P3OUT &= 0x0000;
    P4DIR |= 0x000f;
    P4OUT &= ~BIT0;


    // Timer0
    TA0CCTL0 = CCIE;
    TA0CCR0 = 1000; // 1ms
    TA0CTL = TASSEL_2 + MC_1 + TACLR; // SMCLK : 1Mhz / Up mode to CCR0

    __bis_SR_register(GIE);

    while(1)
    {
    }
        
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
    cnt++;
    if(cnt > 3)
        cnt = 0;
    switch (cnt)
    {
    case 0:
        P3OUT = seg[1]; // 가장 낮은 자리의 데이터
        P4OUT &= ~BIT0;
        P4OUT |= (BIT1|BIT2|BIT3);
        break;
    case 1:
        P3OUT = seg[2]; // 두 번째로 낮은 자리의 데이터
        P4OUT &= ~BIT1;
        P4OUT |= (BIT0|BIT2|BIT3);
        break;
    case 2:
        P3OUT = seg[3]; // 세 번째로 낮은 자리의 데이터
        P4OUT &= ~BIT2;
        P4OUT |= (BIT0|BIT1|BIT3);
        break;
    case 3:
        P3OUT = seg[4]; // 가장 높은 자리의 데이터
        P4OUT &= ~BIT3;
        P4OUT |= (BIT0|BIT1|BIT2);
        break;
    }
}

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

    // 엔코더 Interrupt set
    P1IE |= BIT3;           //Interrupt enabled
    P1IES |= BIT3;          //Interrupt edge(Falling Edge)
    P1IFG &= ~BIT3;         //Interrupt cho_stop
    P1IE |= BIT2;           //Interrupt enabled
    P1IES |= BIT2;          //Interrupt edge(Falling Edge)
    P1IFG &= ~BIT2;         //Interrupt cho_stop

    // LED 설정

    P1DIR |= BIT0; // P1.0을 출력으로 설정
    P4DIR |= BIT7;

    P1OUT &= ~BIT0; //left LED off
    P4OUT &= ~BIT7; //right LED off

    // 스위치
    P1OUT |= BIT1;
    P1REN |= BIT1;
    P1IE |= BIT1;
    P1IES |= BIT1;
    P1IFG &= ~BIT1;

    P2OUT |= BIT1;
    P2REN |= BIT1;
    P2IE |= BIT1;
    P2IES |= BIT1;
    P2IFG &= ~BIT1;

    // 모터
    P2DIR |= (BIT0 | BIT2);     //left DIR : p2.0, right DIR : p2.2
    P2OUT &= ~ (BIT0 | BIT2);   //low

    // PWM
    P2DIR |= (BIT5 | BIT4);
    P2SEL |= (BIT5 | BIT4);

    TA2CCR0 = 1000;             //PWM, Top in Up mode
    TA2CCTL2 = OUTMOD_6;        //PWM toggle/set
    TA2CCR2 = 0;                //left speed
    TA2CCTL1 = OUTMOD_6;        //PWM toggle/set
    TA2CCR1 = 0;                //right speed
    TA2CTL = TASSEL_2 + MC_1;   // SMCLK : 1Mhz / Up mode to CCR0

    // ADC 가변저항 반복 싱글 채널
    P6SEL |= BIT0;
    ADC12CTL0 = ADC12SHT02 + ADC12MSC + ADC12ON; //샘플링 주기 설정 및 ADC ON
    ADC12CTL1 = ADC12SHP + ADC12CONSEQ_2; //샘플링 주기를 설정하기 위한 타이머 설정
    ADC12MCTL0 = ADC12INCH_0; //A0 채널 선택
    ADC12CTL0 |= ADC12ENC; // ADC 활성화
    ADC12CTL0 |= ADC12SC; //데이터 받아오기

    // 초음파 센서
    P2DIR |= BIT7; // trig output으로 설정
    P2OUT &= ~BIT7; // P2.7에 0을 넣겠다.
    P1IE |= BIT4; //P1.4 인터럽트 연결
    P1IES &= ~BIT4; // rising edge로 선택 P1IES |= BIT4;이면 falling edge임
    P1IFG &= ~BIT4; // 인터럽트 플래그 내려줌

    // 키패드
    // output
    P2DIR |= (BIT0|BIT2|BIT3);
    P2OUT |= (BIT0|BIT2|BIT3); // all high

    //input
    P6REN |= (BIT3|BIT4|BIT5|BIT6);
    P6OUT |= (BIT3|BIT4|BIT5|BIT6); // pull up

    // Timer0
    TA0CCTL0 = CCIE;
    TA0CCR0 = 1000; // 1ms
    TA0CTL = TASSEL_2 + MC_1 + TACLR; // SMCLK : 1Mhz / Up mode to CCR0

    // Timer1
    TA1CCTL0 = CCIE;
    TA1CCR0 = 50;
    TA1CTL = TASSEL_2 + MC_1 + TACLR; // TASSEL_1: ACLK : 1Mhz / Up mode to CCR0

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
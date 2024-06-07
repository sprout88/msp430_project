#include <msp430.h>
#define ADC_MIN 1081
#define ADC_MAX 4095
#define ADC_DELTA_TEN_TIME 1507

unsigned int phase = 0; // 문제 번호
unsigned int i = 0;
unsigned int digits[10] = { 0xdb, 0x50, 0x1f, 0x5d, 0xd4, 0xcd, 0xcf, 0xd8, 0xdf, 0xdd}; // 7 segment digits
unsigned int special_digits[] = {
    0x00, /* 0 : 꺼짐 */
    0x20, // 0 : dot */
};
unsigned int screen_arr[4] = {0,};

char seg_select = 0;

unsigned int btn_cool = 0;
unsigned long random_number = 0;
unsigned int random_time = 0;
unsigned int runtime = 0;
unsigned int adc_data = 0;
unsigned long seed = 0;


void main(void){

    // inits

    WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer

    /* p1.1 left switch */
    P1OUT |= BIT1; // DIR
    P1REN |= BIT1; // pull up resister
    P1IE |= BIT1; // Interrupt Enable
    P1IES |= BIT1; // Interrupt edge select : Falling Edge
    P1IFG &= ~BIT1; // interrupt flag

    /* P1.1 right LED */
    P1DIR |= BIT0;
    P1OUT &= ~BIT0; // LED1 OFF

    /* P4.7 left LED */
    P4DIR |= BIT7;
    P4OUT &= ~BIT7; // LED2 OFF

    /* 7 segment Digital Output */
    P3DIR |= 0xffff;
    P3OUT &= 0x0000;
    P4DIR |= 0x000f;
    P4OUT &= ~BIT0;

    /* Timer0 */
    TA0CCTL0 = CCIE;
    TA0CCR0 = 1000; // 1ms;
    TA0CTL = TASSEL_2 + MC_1 + TACLR; // SMCLK : 1Mhz / Up mode to CCRO

    /* Timer1 */
    TA1CCTL0 = CCIE;
    TA1CCR0 = 50;    // 50us
    TA1CTL = TASSEL_2 + MC_1 + TACLR; // SMCLK : 1MHz / Up mode to CCR0

    /* Timer2 */
    TA2CCTL0 = CCIE;
    TA2CCR0 = 5000; // 5ms;
    TA2CTL = TASSEL_2 + MC_1 + TACLR; // SMCLK : 1Mhz / Up mode to CCRO

    /* 모터 */
    P2DIR |= (BIT5 | BIT4);
    P2SEL |= (BIT5 | BIT4);
    TA2CCR0 = 1000;
    TA2CCTL2 = OUTMOD_6;
    TA2CCR2 = 0;
    TA2CCTL1 = OUTMOD_6;
    TA2CCR1 = 0;
    TA2CTL = TASSEL_2 + MC_1;

    /* ADC 가변 저항 */
    P6SEL |= BIT0; // ADC DIR
    ADC12CTL0 = ADC12SHT02 + ADC12MSC + ADC12ON; //ADC REPEAT SINGLE MODE
    ADC12CTL1 = ADC12SHP + ADC12CONSEQ_2; // sample hold : pulse mode, REPEAT SINGLE MODE
    ADC12MCTL0 = ADC12INCH_0; // input channel=A0
    ADC12CTL0 |= ADC12ENC; // ADC12 encoding=enable
    ADC12CTL0 |= ADC12SC; // REPEAT SINGLE MODE

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

    // enable interrupt
    __bis_SR_register(GIE);

    while(1){
        switch(phase){
            case 0: // 스위치 안 누른 처음 상태
                adc_data = ADC12MEM0;
                random_number = (seed*12345)%1000; // 0~1000
                random_time = random_number+1000; // 1000~2000

                break;
            case 1: // 1-1 : 1~2초 랜덤 시간 후에 p1.0 led 1초 점등

                break;
            case 2: // 2-2 : ADC 값 만큼 LED 토글

                break;

        }
    }
}

#pragma vector=PORT1_VECTOR
// left switch p1.1 interrupt
__interrupt void Port_1(void)
{
    if(btn_cool==0){
        if((P1IN & BIT1) == 0)
        {
            phase++;
        }
        btn_cool=1000;
    }
    P1IFG &= ~BIT1; // IFG clear (Interrupt END)
}


#pragma vector=PORT2_VECTOR
// right switch p2.1 interrupt
__interrupt void Port_2(void)
{

    P2IFG &= ~BIT1; // IFG clear
}

//Timer0
#pragma vector=TIMER0_A0_VECTOR // 1ms
__interrupt void TIMER0_A0_ISR(void)
{

}

// Timer1 : Ultrasonic Timer
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void) {

}

// Timer2
#pragma vector=TIMER2_A0_VECTOR
__interrupt void TIMER2_A0_ISR(void)
{
    seg_select++; // 7 Segment Dynamic 구동 타이머
    if (seg_select > 3)
        seg_select = 0; // count 순회

    P3OUT = 0x00;
    switch (seg_select)
    {
    case 0:
        P4OUT &= ~BIT0;
        P4OUT |= (BIT1|BIT2|BIT3);
        P3OUT = screen_arr[0];
        break;

    case 1:
        P4OUT &= ~BIT1;
        P4OUT |= (BIT0|BIT2|BIT3);
        P3OUT = screen_arr[1];
        break;

    case 2:
        P4OUT &= ~BIT2;
        P4OUT |= (BIT0|BIT1|BIT3);
        P3OUT = screen_arr[2];
        break;
    case 3:
        P4OUT &= ~BIT3;
        P4OUT |= (BIT0|BIT1|BIT2);
        P3OUT = screen_arr[3];
        break;
    }
}

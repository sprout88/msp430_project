#include <msp430.h>
#define ADC_MIN 1081
#define ADC_MAX 4095
#define ADC_DELTA_TEN_TIME 1507

unsigned int phase = 0; // 문제 번호
unsigned int digits[10] = {
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
unsigned int special_digits[] = {
    0x00, /* 0 : 꺼짐 */
    0x20, // 0 : dot */
};
unsigned int screen_arr[4] = {0xdb,0x50,0x1f,0xd4};
unsigned int adc_data = 0;
int scaled_adc_data = 0;

int quotient = 0;
int dividend = 0;

unsigned int led_toggle_cool = 0;
char led_left_on = 0;

unsigned int motor_cool = 0;

int total_pwm = 0;
unsigned int clockwise_pwm = 0;
unsigned int anti_clockwise_pwm = 0;

char keypad_pushed[13] = {0,};

char seg_cnt = 0;

unsigned int units = 0; // adc 일의자리
unsigned int tenths_place_num = 0; // adc 십의자리

unsigned int ultrasonic_sec = 0;
char ultrasonic_flag = 0;

void main(void){

    // inits

    WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer

    /* p2.1 right switch */
    P2OUT |= BIT1; // DIR
    P2REN |= BIT1; // pull up resister

    P2IE |= BIT1; // Interrupt Enable
    P2IES |= BIT1; // Interrupt edge select : Falling Edge
    P2IFG &= ~BIT1; // interrupt flag

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

    /* ADC 가변 저항 */
    P6SEL |= BIT0; // ADC DIR
    ADC12CTL0 = ADC12SHT02 + ADC12MSC + ADC12ON; //ADC REPEAT SINGLE MODE
    ADC12CTL1 = ADC12SHP + ADC12CONSEQ_2; // sample hold : pulse mode, REPEAT SINGLE MODE
    ADC12MCTL0 = ADC12INCH_0; // input channel=A0
    ADC12CTL0 |= ADC12ENC; // ADC12 encoding=enable
    ADC12CTL0 |= ADC12SC; // REPEAT SINGLE MODE

    /* 모터 */
    P2DIR |= (BIT5 | BIT4);
    P2SEL |= (BIT5 | BIT4);
    TA2CCR0 = 1000;
    TA2CCTL2 = OUTMOD_6;
    TA2CCR2 = 0;
    TA2CCTL1 = OUTMOD_6;
    TA2CCR1 = 0;
    TA2CTL = TASSEL_2 + MC_1;

    /* 키패드 */
    // output
    P2DIR |= (BIT0 | BIT2 | BIT3);
    P2OUT |= (BIT0 | BIT2 | BIT3); // all high
    // input
    P6REN |= (BIT3 | BIT4 | BIT5 | BIT6);
    P6OUT |= (BIT3 | BIT4 | BIT5 | BIT6); // pull up

    // enable interrupt
    __bis_SR_register(GIE);

    while(1){
        switch(phase){
            case 0: // 스위치 안 누른 처음 상태
                P4OUT |= 0x0f; // xxxx
                break;
            case 1: // 2-1 : ADC 값을 0~2 로 매핑 후 세그먼트 출력
                adc_data = ADC12MEM0;
                dividend = 10 * (adc_data - ADC_MIN);

                if (adc_data < ADC_MIN || adc_data > ADC_MAX) { // out of error 처리
                    scaled_adc_data =  9999; // out of range Error
                }else{
                    scaled_adc_data = dividend / ADC_DELTA_TEN_TIME; // adc 데이터 두자리로 측정완료
                }

                // 화면에 그리기
                units = scaled_adc_data/10%10; // xxN.?xx
                tenths_place_num = scaled_adc_data%10; // xxx.Nxx

                // edit global screen arr
                screen_arr[3] = special_digits[0]; // far left : off
                screen_arr[2] = special_digits[0]; // mid left : off
                screen_arr[1] = digits[units]+special_digits[1]; // mid right : units.
                screen_arr[0] = digits[tenths_place_num]; // far right

                break;
            case 2: // 2-2 : ADC 값 만큼 LED 토글
                if(led_toggle_cool == 0){
                    if(led_left_on){
                        P1OUT |= BIT0; // LED1 ON
                        P4OUT &= ~BIT7; // LED2 OFF
                        led_left_on=0;
                    }else{
                        P4OUT |= BIT7; // LED2 ON
                        P1OUT &= ~BIT0; // LED1 OFF
                        led_left_on=1;
                    }
                    led_toggle_cool = (100*scaled_adc_data);
                }
                break;
            case 3: // 2-3 모터 증감속

                if(motor_cool==0){
                    // pwm 값 받기
                if(keypad_pushed[11]==1){ // * 누름
                    if(clockwise_pwm<1000){
                        if(clockwise_pwm<300){
                            clockwise_pwm = 300; // 초기구간 건너뛰기
                            }
                            clockwise_pwm += 100;
                        }else if(clockwise_pwm>=1000 && anti_clockwise_pwm !=0){ // 1000 보다 커지면 반대쪽 pwm 감소
                            clockwise_pwm = 1000;
                            anti_clockwise_pwm -= 100;
                            if(anti_clockwise_pwm==0){
                                anti_clockwise_pwm=0; // 300까지 감소하면 0으로 점프
                                }
                        }
                        motor_cool = 1000;
                }

                if(keypad_pushed[12]==1){ // # 누름
                    if(anti_clockwise_pwm<1000){
                        if(anti_clockwise_pwm<300){
                            anti_clockwise_pwm = 300;
                            }
                            anti_clockwise_pwm += 100;
                        }else if(anti_clockwise_pwm>=1000 && clockwise_pwm !=0){ // 1000 보다 커지면 반대쪽 pwm 감소
                            anti_clockwise_pwm = 1000;
                            clockwise_pwm -= 100;
                            if(clockwise_pwm==0){
                                clockwise_pwm = 0; // 300 까지 감소하면 0으로 점프
                            }
                        }
                        motor_cool = 1000;
                    }
                }


                /* 키패드 폴링 */
                // Columns 1
                P2OUT &= ~BIT2;
                P2OUT |= (BIT0 | BIT3);
                if ((P6IN & BIT4) == 0 && keypad_pushed[11] == 0) { // Button *
                    keypad_pushed[11] = 1; // Lock the button
                }else if ((P6IN & BIT4) != 0 && keypad_pushed[11] == 1) {
                    keypad_pushed[11] = 0; // Unlock the button
                }
                // Columns 3
                P2OUT &= ~BIT3;
                P2OUT |= (BIT0 | BIT2);
                if ((P6IN & BIT4) == 0 && keypad_pushed[12] == 0) { // Button #
                    keypad_pushed[12] = 1; // Lock the button
                }else if ((P6IN & BIT4) != 0 && keypad_pushed[12] == 1) {
                    keypad_pushed[12] = 0; // Unlock the button
                }


                // total_pwm 계산
                if(clockwise_pwm>anti_clockwise_pwm){
                        total_pwm = clockwise_pwm - anti_clockwise_pwm;
                }else if(clockwise_pwm<anti_clockwise_pwm){
                        total_pwm = clockwise_pwm - anti_clockwise_pwm;
                }else if(clockwise_pwm==anti_clockwise_pwm){
                        total_pwm = 0;
                }

                // 모터 회전
                if(total_pwm>0){
                    TA2CCR2 = total_pwm;
                    TA2CCR1 = 0;
                }else if(total_pwm<0){
                    TA2CCR2 = 0;
                    TA2CCR1 = -total_pwm;
                }else if(total_pwm==0){
                    TA2CCR2 = 0;
                    TA2CCR1 = 0;
                }

                // 키패드 폴링


                break;
            case 4: // 2-4 : 초음파 거리 측정, 물체 감지 시 정지
                break;
        }
    }
}

// right switch interrupt
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
    if((P2IN & BIT1) == 0)
    {
        phase++;
    }
    P2IFG &= ~BIT1; // IFG clear
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
// Ultrasonic Sensor Interrupt
    if (P1IFG & BIT4) {
        if ((P1IES & BIT4) == 0) { // if rising edge
            ultrasonic_sec = 0;
            // TACCTL1 |= TACLR;
            P1IES |= BIT4; // falling edge
        } else if (P1IES & BIT4) { // if falling edge
            if (ultrasonic_sec > 3 && ultrasonic_sec < 500) { // 150us ~ 25ms
                ultrasonic_sec = ultrasonic_sec * 50 / 58; // data = ultrasonic_sec * 50 / 58;
            } else if (ultrasonic_sec >= 760) {
                ultrasonic_sec = 9999;
            }
            ultrasonic_sec = 0;
            P1IES &= ~BIT4; // rising edge
        }
        ultrasonic_flag = 0;
        P1IFG &= ~BIT4; // IFG is cleared
    }
}

//Timer0
#pragma vector=TIMER0_A0_VECTOR // 1ms
__interrupt void TIMER0_A0_ISR(void)
{
    if(led_toggle_cool!=0){
        led_toggle_cool--;
    }
    if(motor_cool!=0){
        motor_cool--;
    }
}

// Timer1 : Ultrasonic Timer
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void) {

}

// Timer2
#pragma vector=TIMER2_A0_VECTOR
__interrupt void TIMER2_A0_ISR(void)
{
    seg_cnt++; // 7 Segment Dynamic 구동 타이머
    if (seg_cnt > 3)
        seg_cnt = 0; // count 순회

    switch (seg_cnt)
    {
    case 0: // ####### 여기서 오류가 발생함. P3OUT 이 case3의 P4OUT 의해 잠깐동안 (1 pc) 출력되면서 잔상이 생김.
        P3OUT = screen_arr[0]; // 여기에 BreakPoint1
        P4OUT &= ~BIT0; // 여기에 BreakPoint2
        P4OUT |= (BIT1|BIT2|BIT3);  // 여기에 BreakPoint3
        break;

    case 1:
        P3OUT = screen_arr[1];
        P4OUT &= ~BIT1;
        P4OUT |= (BIT0|BIT2|BIT3);
        break;

    case 2:
        P3OUT = screen_arr[2];
        P4OUT &= ~BIT2;
        P4OUT |= (BIT0|BIT1|BIT3);
        break;
    case 3:
        P3OUT = screen_arr[3];
        P4OUT &= ~BIT3;
        P4OUT |= (BIT0|BIT1|BIT2);
        break;
    }
}

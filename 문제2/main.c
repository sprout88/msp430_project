#include <msp430.h>
#define ADC_MIN 1081
#define ADC_MAX 4095
#define ADC_DELTA_TEN_TIME 1507

unsigned int phase = 3; // 문제 번호
unsigned int digits[10] = { 0xdb, 0x50, 0x1f, 0x5d, 0xd4, 0xcd, 0xcf, 0xd8, 0xdf, 0xdd}; // 7 segment digits
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

void main(void){

    // inits

    WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer

    /* Left Switch */
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
                    scaled_adc_data =  1111; // out of range Error
                }else{
                    scaled_adc_data = dividend / ADC_DELTA_TEN_TIME; // adc 데이터 두자리로 측정완료
                }

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


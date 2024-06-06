#include <msp430.h>
#define ADC_MIN 1081
#define ADC_MAX 4095
#define ADC_DELTA_TEN_TIME 1507

unsigned int phase = 1; // 문제 번호
unsigned int digits[10] = { 0xdb, 0x50, 0x1f, 0x5d, 0xd4, 0xcd, 0xcf, 0xd8, 0xdf, 0xdd}; // 7 segment digits
unsigned int screen_arr[4] = {0xdb,0x50,0x1f,0xd4};
unsigned int adc_data = 0;
int scaled_adc_data = 0;

int quotient = 0;
int dividend = 0;

unsigned int led_toggle_cool = 0;
char led_left_on = 0;

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
    TA0CTL = TASSEL_2 + MC_1 + TACLR; // SMCLK : 1Mhz / Up mode to CCRO'

    /* ADC 가변 저항 */
    P6SEL |= BIT0; // ADC DIR
    ADC12CTL0 = ADC12SHT02 + ADC12MSC + ADC12ON; //ADC REPEAT SINGLE MODE
    ADC12CTL1 = ADC12SHP + ADC12CONSEQ_2; // sample hold : pulse mode, REPEAT SINGLE MODE
    ADC12MCTL0 = ADC12INCH_0; // input channel=A0
    ADC12CTL0 |= ADC12ENC; // ADC12 encoding=enable
    ADC12CTL0 |= ADC12SC; // REPEAT SINGLE MODE

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
}


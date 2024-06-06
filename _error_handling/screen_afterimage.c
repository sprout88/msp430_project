// 테스트 코드에서는 잔상 없이 잘 작동한다.

#include <msp430.h>

/* datas */
unsigned int digits[10] = { 0xdb, 0x50, 0x1f, 0x5d, 0xd4, 0xcd, 0xcf, 0xd8, 0xdf, 0xdd}; // 7 segment digits
unsigned int special_digits[] = {
    0x00, /* 0 : 꺼짐 */
    0x20, // 0 : dot */
};

unsigned int screen_arr[4] = {0xdb,0x50,0x00,0x00};
unsigned int dynamic_segment_cnt = 0; // iterate 0~3
char p4_7_left_led_on = 0; // led and screen error fix

void stop_watchdog_timer(void);
void init_7_segment(void);
void init_timer0(void);
void show_screen_arr(void);
void enable_interrupt_vector(void);

// ### Main ###
void main(void) {
    stop_watchdog_timer();
    init_timer0();
    init_7_segment();

    enable_interrupt_vector();
    while(1){

    }
}

void stop_watchdog_timer(void){
    WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer
}

void init_7_segment(void){
    /* 7 segment Digital Output */
    P3DIR |= 0xffff;
    P3OUT &= 0x0000;
    P4DIR |= 0x000f;
    P4OUT = 0x0F; // segment : XXXX
}

void init_timer0(void){
    /* Timer - Timer0 */
    TA0CCTL0 = CCIE;
    TA0CCR0 = 5000; //1000;
    TA0CTL = TASSEL_2 + MC_1 + TACLR; // SMCLK : 1Mhz / Up mode to CCRO
    /* END Timer - Timer0 */
}

void enable_interrupt_vector(void){
    __bis_SR_register(GIE);
}

void show_screen_arr(void){
    if (dynamic_segment_cnt > 3)
            dynamic_segment_cnt = 0; // count 순회

    switch (dynamic_segment_cnt)
    {
    case 0:
        P3OUT = screen_arr[0];
        P4OUT = 0x8E;
        break;
    case 1:
        P3OUT = screen_arr[1];
        P4OUT = 0x8D;
        break;
    case 2:
        P3OUT = screen_arr[2];
        P4OUT = 0x8B;
        break;
    case 3:
        P3OUT = screen_arr[3];
        P4OUT = 0x87;
        break;
    }
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
    dynamic_segment_cnt++; // 7 Segment Dynamic 구동 타이머
    if (dynamic_segment_cnt > 3) {
        dynamic_segment_cnt = 0; // count 순회
    }
    show_screen_arr(); // show adc_data
}

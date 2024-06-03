#include <msp430.h>

unsigned int digits[10] = { 0xdb, 0x50, 0x1f, 0x5d, 0xd4, 0xcd, 0xcf, 0xd8, 0xdf, 0xdd}; // 7 segment digits
unsigned int adc_data = 4143;
unsigned int dynamic_segment_cnt = 0;

/* watchdog timer functions */
void stop_watchdog_timer(void);

/* switch functions */
void init_right_switch(void);
void init_left_switch(void);

/* 7 segment functions */
void init_7_segment(void);
void show_screen(unsigned int);
void init_dynamic_timer(void);

/* ADC functions */
void init_ADC_single_mode(void);
void ADC_single_read(unsigned int* p_data);

/* interrupt functions */
void enable_interrupt_vector(void);

void main(void) {
    // Write Your Code Here
}

void init_right_switch(void){
    P1OUT |= BIT1; // DIR
    P1REN |= BIT1; // pull up resister

    P1IE |= BIT1; // Interrupt Enable
    P1IES |= BIT1; // Interrupt edge select : Falling Edge
    P1IFG &= ~BIT1; // interrupt flag
}

void init_left_switch(void){
    P2OUT |= BIT1; // DIR
    P2REN |= BIT1; // pull up resister

    P2IE |= BIT1; // Interrupt Enable
    P2IES |= BIT1; // Interrupt edge select : Falling Edge
    P2IFG &= ~BIT1; // interrupt flag
}

void init_7_segment(void){
    /* 7 segment Digital Output */
    P3DIR |= 0xffff;
    P3OUT &= 0x0000;
    P4DIR |= 0x000f;
    P4OUT &= ~BIT0;
    /* END 7 segment Digital Output */
}
void init_dynamic_timer(void){
    /* Timer - Timer0 */
    TA0CCTL0 = CCIE;
    TA0CCR0 = 5000; //1000;
    TA0CTL = TASSEL_2 + MC_1 + TACLR; // SMCLK : 1Mhz / Up mode to CCRO
    /* END Timer - Timer0 */
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

void enable_interrupt_vector(void){
    __bis_SR_register(GIE);
}

// Timer interrupt service routine
// 7 Segment Dinamic 구동 타이머
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
    dynamic_segment_cnt++;
    show_screen(adc_data);
}
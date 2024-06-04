#include <msp430.h>
#define ADC_DELTA_TEN_TIME 1507
#define ADC_MIN 1081
#define ADC_MAX 4095

unsigned int digits[10] = { 0xdb, 0x50, 0x1f, 0x5d, 0xd4, 0xcd, 0xcf, 0xd8, 0xdf, 0xdd}; // 7 segment digits
unsigned int special_digits[] = {
    0x20, /* dot */
};
unsigned int screen_arr[4] = {0xdb,0xdb,0xdb,0xdb};
unsigned int adc_data = 3000;
unsigned int dynamic_segment_cnt = 0; // iterate 0~3
unsigned int smclk_cnt = 0; // iterate 0ms ~ 1000ms
unsigned int sec_cnt = 0; // iterate 1sec ~ 65535sec
unsigned int tmp1 = 0;

unsigned int is_left_switch = 0;
unsigned int is_right_switch = 0;
unsigned int screen_mode = 0; // 0: arr_mode, 1: decimal mode

/* watchdog timer functions */
void stop_watchdog_timer(void);

/* switch functions */
void init_right_switch(void);
void init_left_switch(void);
void right_switch_interrupt_handler(void);
void left_switch_interrupt_handler(void);

/* 7 segment functions */
void init_7_segment(void);
void init_smclk(void);
void show_screen(unsigned int);
void show_screen_arr();


/* ADC functions */
void init_ADC_single_mode(void);
void init_ADC_repeat_single_mode(void);
void ADC_single_read(unsigned int* p_data);
void ADC_repeat_single_read(unsigned int* p_data);
void adc_to_segment(void);

/* interrupt functions */
void enable_interrupt_vector(void);

/* custom functions for solve projects */
unsigned int scale_transform(int input);

////////////////////////////////////


// ### Original Functions Prototypes

// ### Main ###
void main(void) {
    stop_watchdog_timer();

    init_smclk();

    init_7_segment();
    init_right_switch();
    init_left_switch();
    init_ADC_single_mode();

    enable_interrupt_vector();

    while(1){
        screen_arr[3] = digits[sec_cnt];
    }
}

// ### Original Function declare
/* Write Your Function Here */

// ### override functions

// right switch dir p2.1
void right_switch_interrupt_handler(void){
    adc_to_segment();
}

// left switch dir p1.1
void left_switch_interrupt_handler(void){
    // write your code here
}

///////////////////////////////////////

//
// ### non-override functions

void adc_to_segment(void){
    ADC_single_read(&adc_data);
    int scaled_adc_data = scale_transform(adc_data);
    if(scaled_adc_data != 1111){
        unsigned int units = scaled_adc_data/10%10; // ??N.???
        unsigned int tenths_place_num = scaled_adc_data%10; // ???.N??
        screen_arr[3] = digits[0]; // far left
        screen_arr[2] = digits[units]; // mid left
        screen_arr[1] = special_digits[0]; // mid right
        screen_arr[0] = digits[tenths_place_num]; // far right
    }
}

void stop_watchdog_timer(void){
    WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer
}

void init_left_switch(void){
    P1OUT |= BIT1; // DIR
    P1REN |= BIT1; // pull up resister

    P1IE |= BIT1; // Interrupt Enable
    P1IES |= BIT1; // Interrupt edge select : Falling Edge
    P1IFG &= ~BIT1; // interrupt flag
}

void init_right_switch(void){
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
}

void init_smclk(void){
    /* Timer - Timer0 */
    TA0CCTL0 = CCIE;
    TA0CCR0 = 1000; //1000;
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

void show_screen_arr(){
    if (dynamic_segment_cnt > 3)
            dynamic_segment_cnt = 0; // count 순회

    switch (dynamic_segment_cnt)
    {
    case 0:
        P3OUT = screen_arr[0];
        P4OUT = ~BIT0;
        break;
    case 1:
        P3OUT = screen_arr[1];
        P4OUT = ~BIT1;
        break;
    case 2:
        P3OUT = screen_arr[2];
        P4OUT = ~BIT2;
        break;
    case 3:
        P3OUT = screen_arr[3];
        P4OUT = ~BIT3;
        break;
    }
}

unsigned int scale_transform(int input) {
    if (input < ADC_MIN || input > ADC_MAX) {
        return 1111; // out of range Error
    }

    int quotient = 0;
    int dividend = 10 * (input - ADC_MIN);

    while ((dividend - ADC_DELTA_TEN_TIME) >= 0) {
        dividend -= ADC_DELTA_TEN_TIME;
        quotient++;
    }
    return quotient;
}

void init_ADC_single_mode(void){
    P6SEL |= BIT0; // ADC DIR

    /* ADC12 control register set */
    /* sample hold time : 16 adc clock cycles, ADC12 ON */
    ADC12CTL0 = ADC12SHT02 + ADC12ON;

    ADC12CTL1 = ADC12SHP; // sample hold : pulse mode
    ADC12MCTL0 = ADC12INCH_0; // input channel=A0
    ADC12CTL0 |= ADC12ENC; // ADC12 encoding=enable
}

void init_ADC_repeat_single_mode(void){
    P6SEL |= BIT0; // ADC DIR

    /* ADC12 control register set */
    /* sample hold time : 16 adc clock cycles, ADC12 ON */

    ADC12CTL0 = ADC12SHT02 + ADC12MSC + ADC12ON; //ADC REPEAT SINGLE MODE
    ADC12CTL1 = ADC12SHP + ADC12CONSEQ_2; // sample hold : pulse mode, REPEAT SINGLE MODE
    ADC12MCTL0 = ADC12INCH_0; // input channel=A0
    ADC12CTL0 |= ADC12ENC; // ADC12 encoding=enable

    ADC12CTL0 |= ADC12SC; // REPEAT SINGLE MODE
}


void ADC_single_read(unsigned int* p_data){
    ADC12CTL0 |= ADC12SC; // ADC control register set
    while(!(ADC12IFG & BIT0)); // prevent reading previous data
    *p_data = ADC12MEM0; // data save
}

void ADC_repeat_single_read(unsigned int* p_data){
    *p_data = ADC12MEM0;
}

void enable_interrupt_vector(void){
    __bis_SR_register(GIE);
}

// Timer interrupt service routine
// 1ms 마다 호출됨
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
    dynamic_segment_cnt++; // 7 Segment Dynamic 구동 타이머
    smclk_cnt++; // 1++ per 1ms
    if(smclk_cnt>1000){
        sec_cnt++;
        smclk_cnt=0;
    }
    switch(screen_mode){
        case 0:
            show_screen_arr();
            break;
        case 1:
            show_screen(adc_data);
            break;
    }
}

// right switch interrupt
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
    if((P2IN & BIT1) == 0)
    {
        is_right_switch = 1;
        right_switch_interrupt_handler();
    }
    P2IFG &= ~BIT1; // IFG clear (Interrupt END)
}

// left switch interrupt
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    if((P1IN & BIT1) == 0)
    {
        is_left_switch = 1;
        left_switch_interrupt_handler();
    }
    P1IFG &= ~BIT1; // IFG clear (Interrupt END)
}

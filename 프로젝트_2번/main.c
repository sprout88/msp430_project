#include <msp430.h>
#define ADC_DELTA_TEN_TIME 1507
#define ADC_MIN 1081
#define ADC_MAX 4095

unsigned int digits[10] = { 0xdb, 0x50, 0x1f, 0x5d, 0xd4, 0xcd, 0xcf, 0xd8, 0xdf, 0xdd}; // 7 segment digits
unsigned int special_digits[] = {
    0x20, /* dot */
};
unsigned int screen_arr[4] = {0xdb,0x50,0x1f,0x5d};
unsigned int adc_data = 3000;
unsigned int dynamic_segment_cnt = 0; // iterate 0~3
unsigned int smclk_cnt = 0; // iterate 0ms ~ 1000ms
unsigned int sec_cnt = 0; // iterate 1sec ~ 65535sec
unsigned int tmp1 = 0;
unsigned int scaled_adc_data = 0;
unsigned int ms_timer_1 = 0;
unsigned int toggle_lock = 0; // 0 : off, 1 : on
unsigned int is_left_switch = 0;
unsigned int is_right_switch = 0;
unsigned int screen_mode = 0; // 0: arr_mode, 1: decimal mode
unsigned int led_toggle_state = 0;

char p4_7_left_led_on = 0; // led and screen error fix

/* watchdog timer functions */
void stop_watchdog_timer(void);

/* switch functions */
void init_right_switch(void);
void init_left_switch(void);
void right_switch_interrupt_handler(void);
void left_switch_interrupt_handler(void);

/* led functions */
void init_led(int led_num);
void turn_on_led(int led_num);
void turn_off_led(int led_num);
void toggle_led_per_time(unsigned int toggle_interval_ms);

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
void scaled_adc_to_segment_arr(void);

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

    init_led(1);
    init_led(2);

    enable_interrupt_vector();

    while(1){
        toggle_led_per_time(3000); // only if toggle_lock = true
        show_screen_arr(); // show adc_data
    }
}

// ### Original Function declare
/* Write Your Function Here */

// ### override functions

// right switch dir p2.1
void right_switch_interrupt_handler(void){
    scaled_adc_to_segment_arr();
    toggle_lock ^= 1;
}

// left switch dir p1.1
void left_switch_interrupt_handler(void){
    // write your code here
}

///////////////////////////////////////

//
// ### non-override functions

void scaled_adc_to_segment_arr(void){
    ADC_single_read(&adc_data);
    scaled_adc_data = scale_transform(adc_data);
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
    P4OUT = 0x0F; // segment : XXXX
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
        if(p4_7_left_led_on) // segment : XXXO
            P4OUT = 0x8E;
        else
            P4OUT = 0x0E;
        break;
    case 1:
        P3OUT = digits[value/10%10];
        if(p4_7_left_led_on) // segment : XXOX
            P4OUT = 0x8D;
        else
            P4OUT = 0x0D;
        break;
    case 2:
        P3OUT = digits[value/100%10];
        if(p4_7_left_led_on) // segment : XOXX
            P4OUT = 0x8B;
        else
            P4OUT = 0x0B;
        break;
    case 3:
        P3OUT = digits[value/1000%10];
        if(p4_7_left_led_on) // segment : OXXX
            P4OUT = 0x87;
        else
            P4OUT = 0x07;
        break;
    }
}

void show_screen_arr(void){
    if (dynamic_segment_cnt > 3)
            dynamic_segment_cnt = 0; // count 순회

    switch (dynamic_segment_cnt)
    {
    case 0:
        P3OUT = screen_arr[0];
        if(p4_7_left_led_on) // segment : XXXO
            P4OUT = 0x8E;
        else
            P4OUT = 0x0E;
        break;
    case 1:
        P3OUT = screen_arr[1];
        if(p4_7_left_led_on) // segment : XXOX
            P4OUT = 0x8D;
        else
            P4OUT = 0x0D;
        break;
    case 2:
        P3OUT = screen_arr[2];
        if(p4_7_left_led_on) // segment : XOXX
            P4OUT = 0x8B;
        else
            P4OUT = 0x0B;
        break;
    case 3:
        P3OUT = screen_arr[3];
        if(p4_7_left_led_on) // segment : OXXX
            P4OUT = 0x87;
        else
            P4OUT = 0x07;
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

void toggle_led_per_time(unsigned int toggle_interval_ms){
    if(toggle_lock==1){
        // toggle two led per time
        if(ms_timer_1 > toggle_interval_ms ){
            // toggle leds
            if(led_toggle_state){
                turn_on_led(1);
                turn_off_led(2);
                led_toggle_state=0;
            }else{
                turn_off_led(1);
                turn_on_led(2);
                led_toggle_state=1;
            }
            ms_timer_1 = 0;
        }
    }
}

void init_led(int led_num){
    switch(led_num){
        case 1:
            // LED1 DIR : P1.1
            P1DIR |= BIT0;
            // LED1 OFF
            P1OUT &= ~BIT0;
            break;
        case 2:
            // LED2 DIR : P4.7
            P4DIR |= BIT7;
            // LED2 OFF
            P4OUT &= ~BIT7; // LED2 OFF
    }
}

void turn_on_led(int led_num){
    switch(led_num){
        case 1:
            P1OUT |= BIT0; // LED1 ON
            break;
        case 2:
            P4OUT |= BIT7; // LED2 ON
            p4_7_left_led_on = 1; // p4.7 led and segment pin duplicate error fix
            break;
    }
}
void turn_off_led(int led_num){
    switch(led_num){
        case 1:
            P1OUT &= ~BIT0; // LED1 OFF
            break;
        case 2:
            P4OUT &= ~BIT7; // LED2 OFF
            p4_7_left_led_on = 0; // p4.7 led and segment pin duplicate error fix
            break;
    }
}
void toggle_led(int led_num){
    switch(led_num){
        case 1:
            P1OUT ^= BIT0; // LED1 TOGGLE
        case 2:
            P4OUT ^= BIT7; // LED2 TOGGLE
    }
}


// Timer interrupt service routine
// 1ms 마다 호출됨
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
    dynamic_segment_cnt++; // 7 Segment Dynamic 구동 타이머
    smclk_cnt++; // 1++ per 1ms, iterate
    ms_timer_1++; // 1++ per 1ms, no iterate
    if(smclk_cnt>1000){ // 1초를 셈
        sec_cnt++;
        smclk_cnt=0;
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

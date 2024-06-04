#include <msp430.h>
#define ADC_DELTA_TEN_TIME 1507
#define ADC_MIN 1081
#define ADC_MAX 4095

unsigned int tmp1 = 0;
unsigned int tmp2 = 0;

unsigned int digits[10] = { 0xdb, 0x50, 0x1f, 0x5d, 0xd4, 0xcd, 0xcf, 0xd8, 0xdf, 0xdd}; // 7 segment digits
unsigned int special_digits[] = {
    0x20, /* dot */
};
unsigned int screen_arr[4] = {0xdb,0x50,0x1f,0x5d};
unsigned int adc_data = 3000;
unsigned int scaled_adc_data = 0;

/* timers */
unsigned int led_toggle_cnt = 0;
unsigned int motor_cnt_7 = 0; // 문제 2-3 에서 모터 7초 세기 카운터
char motor_cnt_7_lock = 0; // counter lock, enable(1) 일때만 카운트. lock(0)
unsigned int dynamic_segment_cnt = 0; // iterate 0~3
unsigned int smclk_cnt = 0; // iterate 0ms ~ 1000ms

/* locks */
unsigned int toggle_lock = 0; // 0 : off, 1 : on
unsigned int is_left_switch = 0;
unsigned int is_right_switch = 0;
unsigned int screen_mode = 0; // 0: arr_mode, 1: decimal mode
unsigned int led_toggle_state = 0;
char p4_7_left_led_on = 0; // led and screen error fix

unsigned int g_anti_clockwise_pwm = 0;
unsigned int g_clockwise_pwm = 0;

/* motor locks */
unsigned int g_motor_clockwise_spin_start = 0;
unsigned int g_motor_anti_clockwise_spin_start = 0;

int g_motor_signal = 0; // 0:(신호없음), 1:on명령, -1:off명령
int g_motor_spin_direction_signal = 0; // -1:anti-clockwise, 0:stop, 1:clock-wise

int g_motor_toggle = 0; // 0:off, 1:on

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
void toggle_led_per_time_ms(unsigned int toggle_interval_ms);

/* 7 segment functions */
void init_7_segment(void);
void init_smclk(void);
void show_screen(unsigned int);
void show_screen_arr(void);

/* keypad functions */
void init_keypad(void);
void keypad_input_polling_checker(void);
void keypad_push_handler(unsigned int key);
void keypad_release_handler(unsigned int key);

/* motor and encoder functions */
void init_motor(void);
void set_motor_spin_pwm(unsigned int clockwise_pwm, unsigned int anti_clockwise_pwn);\
void motor_speed_controller_7(int clockwise, unsigned int* p_cnt_7); // set_motor_spin_pwm 과 같이 사용해야함

/* ADC functions */
void init_ADC_single_mode(void);
void init_ADC_repeat_single_mode(void);
void ADC_single_read(unsigned int* p_data); // read adc hardware and save to global_var:adc_data
void ADC_repeat_single_read(unsigned int* p_data);
void adc_data_scale_and_save_to_segment_arr(unsigned int* p_adc_data, unsigned int* p_screen_arr);

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

    init_keypad();

    init_motor();

    enable_interrupt_vector();

    while(1){
        toggle_led_per_time_ms(scaled_adc_data*100); // only if toggle_lock = true, scaled_adc_data(0~20)
        show_screen_arr(); // show adc_data
        keypad_input_polling_checker();
        set_motor_spin_pwm(g_anti_clockwise_pwm,g_clockwise_pwm); // 모터 회전, switch interrupt handler 에 의해 global_pwm 변경으로 회전 조정
        motor_speed_controller_7(g_motor_spin_direction_signal, &motor_cnt_7); // set_motor_spin_pwm 과 같이 사용해야함
    }
}

// ### Original Function declare
/* Write Your Function Here */

// ### override functions

// right switch dir p2.1
void right_switch_interrupt_handler(void){
    ADC_single_read(&adc_data); // read adc hardware and save to global_var:adc_data
    adc_data_scale_and_save_to_segment_arr(&adc_data,&screen_arr[0]); // convert adc_data to special scaled formet and save to segment_arr

    if(toggle_lock == 1){ // adc_scaled_sec 로 led를 이미 toggle 중이였다면 종료 후 모터 모드로 전환

    }

    toggle_lock = 1;

}

// left switch dir p1.1
void left_switch_interrupt_handler(void){
    // write your code here
    g_anti_clockwise_pwm = 500;
    g_clockwise_pwm = 0;
}

void keypad_push_handler(unsigned int key){ // 각 case 를 구현하지 않아도 오류 없음. (keypad_input_polling_checker에서 검증함)
    switch(key){
        case 0:
            tmp1=0;
            // input your handler
            break;
        case 1:
            tmp1=1;
            // input your handler
            break;
        case 2:
            tmp1=2;
            // input your handler
            break;
        case 3:
            tmp1=3;
            // input your handler
            break;
        case 4:
            tmp1=4;
            // input your handler
            break;
        case 5:
            tmp1=5;
            // input your handler
            break;
        case 6:
            tmp1=6;
            // input your handler
            break;
        case 7:
            tmp1=7;
            // input your handler
            break;
        case 8:
            tmp1=8;
            // input your handler
            break;
        case 9:
            tmp1=9;
            // input your handler
            break;
        case 10:
            tmp1=10;
            // input your handler
            break;
        case 11: // 11:star
            tmp1+=1;
            if(g_motor_toggle==0){
                g_motor_toggle=1; // motor toggle UP

                // message passing
                g_motor_spin_direction_signal = -1; // anti-clockwise
                g_motor_signal=1; // motor on signal

            }else{ // 모터가 돌아가고 있을때 버튼 누르면 모터 정지 명령
                g_motor_toggle=0; // motor toggle DOWN

                // mesasage passing
                g_motor_spin_direction_signal = 0; // direction none signal
                g_motor_signal=-1; // motor off signal
            }
            break;
        case 12: // 12:sharp
            tmp1=12;
            if(g_motor_toggle==0){
                g_motor_toggle=1; // motor toggle UP

                // message passing
                g_motor_spin_direction_signal = 1; // clockwise
                g_motor_signal=1; // motor on signal

            }else{ // 모터가 돌아가고 있을때 버튼 누르면 모터 정지 명령
                g_motor_toggle=0; // motor toggle DOWN

                // mesasage passing
                g_motor_spin_direction_signal = 0; // direction none signal
                g_motor_signal=-1; // motor off signal
            }
            break;
    }
}
void keypad_release_handler(unsigned int key){ // 각 case 를 구현하지 않아도 오류 없음. (keypad_input_polling_checker에서 검증함)
    switch(key){
        case 0:
            tmp2=0;
            // input your handler
            break;
        case 1:
            tmp2=1;
            // input your handler
            break;
        case 2:
            tmp2=2;
            // input your handler
            break;
        case 3:
            tmp2=3;
            // input your handler
            break;
        case 4:
            tmp2=4;
            // input your handler
            break;
        case 5:
            tmp2=5;
            // input your handler
            break;
        case 6:
            tmp2=6;
            // input your handler
            break;
        case 7:
            tmp2=7;
            // input your handler
            break;
        case 8:
            tmp2=8;
            // input your handler
            break;
        case 9:
            tmp2=9;
            // input your handler
            break;
        case 10:
            tmp2=10;
            // input your handler
            break;
        case 11: // 11:star
            tmp2=11;
            g_anti_clockwise_pwm++;
            break;
        case 12: // 12:sharp
            tmp2=12;
            g_clockwise_pwm++;
            break;
    }
}
///////////////////////////////////////

//
// ### non-override functions

void adc_data_scale_and_save_to_segment_arr(unsigned int* p_adc_data, unsigned int* p_screen_arr){
    scaled_adc_data = scale_transform(*p_adc_data);
    if(scaled_adc_data != 1111){
        unsigned int units = scaled_adc_data/10%10; // ??N.???
        unsigned int tenths_place_num = scaled_adc_data%10; // ???.N??
        p_screen_arr[3] = digits[0]; // far left
        p_screen_arr[2] = digits[units]; // mid left
        p_screen_arr[1] = special_digits[0]; // mid right
        p_screen_arr[0] = digits[tenths_place_num]; // far right
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

void toggle_led_per_time_ms(unsigned int toggle_interval_ms){
    if(toggle_lock==1){
        // toggle two led per time
        if(led_toggle_cnt > toggle_interval_ms ){
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
            led_toggle_cnt = 0;
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

void init_keypad(void){
    // output
    P2DIR |= (BIT0 | BIT2 | BIT3);
    P2OUT |= (BIT0 | BIT2 | BIT3); // all high

    // input
    P6REN |= (BIT3 | BIT4 | BIT5 | BIT6);
    P6OUT |= (BIT3 | BIT4 | BIT5 | BIT6); // pull up
}

// keypad handler
void keypad_input_polling_checker(void){
    // columns 1
    P2OUT &= ~BIT2;
    P2OUT |= (BIT0 | BIT3);

    if ((P6IN & BIT3) == 0) // Button 1
    {
        while(((P6IN & BIT3) == 0)){
            keypad_push_handler(1);
        }
        keypad_release_handler(1);
    }
    else if ((P6IN & BIT6) == 0) // Button 4
    {
        while(((P6IN & BIT6) == 0)){
            keypad_push_handler(4);
        }
        keypad_release_handler(4);
    }
    else if ((P6IN & BIT5) == 0) // Button 7
    {
        while(((P6IN & BIT5) == 0)){
            keypad_push_handler(7);
        }
        keypad_release_handler(7);
    }
    else if ((P6IN & BIT4) == 0) // Button *
    {
        while(((P6IN & BIT4) == 0)){
            keypad_push_handler(11);
        }
        keypad_release_handler(11); //11:*
    }

    // columns 2
    P2OUT &= ~BIT0;
    P2OUT |= (BIT2 | BIT3);

    if ((P6IN & BIT3) == 0) // Button 2
    {
        while(((P6IN & BIT3) == 0)){
            keypad_push_handler(2);
        }
        keypad_release_handler(2);
    }
    else if ((P6IN & BIT6) == 0) // Button 5
    {
        while(((P6IN & BIT6) == 0)){
            keypad_push_handler(5);
        }
        keypad_release_handler(5);
    }
    else if ((P6IN & BIT5) == 0) // Button 8
    {
        while(((P6IN & BIT5) == 0)){
            keypad_push_handler(8);
        }
        keypad_release_handler(8);
    }
    else if ((P6IN & BIT4) == 0) // Button 0
    {
        while(((P6IN & BIT4) == 0)){
            keypad_push_handler(0);
        }
        keypad_release_handler(0);
    }

    // columns 3
    P2OUT &= ~BIT3;
    P2OUT |= (BIT0 | BIT2);

    if ((P6IN & BIT3) == 0) // Button 3
    {
        while(((P6IN & BIT3) == 0)){
            keypad_push_handler(3);
        }
        keypad_release_handler(3);
    }
    else if ((P6IN & BIT6) == 0) // Button 6
    {
        while(((P6IN & BIT6) == 0)){
            keypad_push_handler(6);
        }
        keypad_release_handler(6);
    }
    else if ((P6IN & BIT5) == 0) // Button 9
    {
        while(((P6IN & BIT5) == 0)){
            keypad_push_handler(9);
        }
        keypad_release_handler(9);
    }
    else if ((P6IN & BIT4) == 0) // Button #
    {
        while(((P6IN & BIT4) == 0)){
            keypad_push_handler(12);
        }
        keypad_release_handler(12); // 12:sharp

    }
}

// motor functions
void init_motor(void){
    // PWN set
    P2DIR |= (BIT5 | BIT4);
    P2SEL |= (BIT5 | BIT4);
    TA2CCR0 = 1000;
    TA2CCTL2 = OUTMOD_6;
    TA2CCR2 = 0;
    TA2CCTL1 = OUTMOD_6;
    TA2CCR1 = 0;
    TA2CTL = TASSEL_2 + MC_1;
}
void set_motor_spin_pwm(unsigned int clockwise_pwm, unsigned int anti_clockwise_pwn){
    TA2CCR2 = clockwise_pwm;
    TA2CCR1 = anti_clockwise_pwn;
}
void motor_speed_controller_7(int dir_signal_recved, unsigned int* p_cnt_7){ // set_motor_spin_pwm 과 같이 사용해야함
    unsigned int interpolated_pwm = 0;

    if(g_motor_signal==1) // 모터 가동 신호를 받으면 모터를 작동시킴
    {
        if(motor_cnt_7_lock==0)
            motor_cnt_7_lock=1; // 카운터락 해제, 7초 세기 시작

        if(*p_cnt_7<7000){ // 모터 7초 동안 속력 서서히 증가
            interpolated_pwm = *p_cnt_7/10 + 300; // 선형보간으로 계산, 0sec->300pwm,7000sec->1000pwm
            if(dir_signal_recved)
                g_clockwise_pwm = interpolated_pwm;
            else
                g_anti_clockwise_pwm = interpolated_pwm;
            }else{
                //7초 종료 후에는 최고속 유지
                *p_cnt_7=0; // 7초 카운터 초기화
                motor_cnt_7_lock=0; // 7초 카운터 잠금
                g_motor_signal = 0; // signal 초기화
            }
    }else if(g_motor_signal==-1) // 모터 정지 신호를 받으면 모터를 정지시킴
    {
        if(motor_cnt_7_lock==0)
            motor_cnt_7_lock=1; // 7초 세기 시작

        if(*p_cnt_7<7000){ // 7초 동안 속력 서서히 감소
            interpolated_pwm = *p_cnt_7/10 + 300;
            if(dir_signal_recved) // 방향에 따라 다르게 작동
                g_clockwise_pwm = interpolated_pwm;
            else
                g_anti_clockwise_pwm = interpolated_pwm;
        }else{
            // 7초 뒤
            *p_cnt_7=0; // 7초 카운터 초기화
            motor_cnt_7_lock=0; // 7초 카운터 잠금

            /* 두 모터 정지 */
            g_clockwise_pwm = 0;
            g_anti_clockwise_pwm = 0;

            // 상태를 정지로 바꿈.
            g_motor_signal = 0; // signal 초기화
        }

    }
}
// Timer interrupt service routine
// 1ms 마다 호출됨
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
    /* overflow counters */
    // 0~65535 까지 증가하고 0으로 초기화됨
    dynamic_segment_cnt++; // 7 Segment Dynamic 구동 타이머
    led_toggle_cnt++; // 1++ per 1ms, no iterate

    /* iterate counters */
    // 특정값까지 세고 0으로 돌아옴
    smclk_cnt++; // 1++ per 1ms, iterate
    if(smclk_cnt>1000){ // 1초를 셈 (1ms)
        smclk_cnt=0;
    }

    /* lock counters */
    // enable(1) 되었을때만 카운트.
    if(motor_cnt_7_lock==1)
        motor_cnt_7++; // 1++ per 1ms, iterate
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

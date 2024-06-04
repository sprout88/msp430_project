/* 워치독 타이머 전역변수 오류 */

/* 
 1. watchdog_timer 를 끄지 않은 상태에서, 전역변수를 함수로 바꿔도 일정 클럭 이후 초기화 되지는 않는다.
 2. watchdog_timer 를 끄지 않은 상태에서, 전역변수를 interrupt 로 바꾸면 일정 클럭 이후 초기화 된다.

*/

#include <msp430.h>
#define ADC_DELTA_TEN_TIME 1507
#define ADC_MIN 1081
#define ADC_MAX 4095

unsigned int temp = 4;
unsigned int temp_for_interrupt = 12;

/* watchdog timer functions */

void stop_watchdog_timer(void);

/* switch functions */
void init_right_switch(void);
void right_switch_interrupt_handler(void);
void change_temp(void);


/* interrupt functions */
void enable_interrupt_vector(void);

////////////////////////////////////


// ### Original Functions Prototypes

// ### Main ###
void main(void) {
    //stop_watchdog_timer(); // 워치독 타이머 끄기. 주석 시 켜져있음
    init_right_switch();
    enable_interrupt_vector();

    change_temp();

    while(1){

    }
}

void change_temp(void){
    temp=882;
}


///////////////////////////////////////

//
// ### non-override functions
void stop_watchdog_timer(void){
    WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer
}

void enable_interrupt_vector(void){
    __bis_SR_register(GIE);
}

void init_right_switch(void){
    P2OUT |= BIT1; // DIR
    P2REN |= BIT1; // pull up resister

    P2IE |= BIT1; // Interrupt Enable
    P2IES |= BIT1; // Interrupt edge select : Falling Edge
    P2IFG &= ~BIT1; // interrupt flag
}

void right_switch_interrupt_handler(void){
    temp_for_interrupt = 94;
}


// right switch interrupt
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
    if((P2IN & BIT1) == 0)
    {
        right_switch_interrupt_handler();
    }
    P2IFG &= ~BIT1; // IFG clear (Interrupt END)
}


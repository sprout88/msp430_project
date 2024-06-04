/* 워치독 타이머 전역변수 오류 */

/* 
 1. watchdog_timer 를 끄지 않은 상태에서, 전역변수를 함수로 바꿔도 일정 클럭 이후 초기화 되지는 않는다.
 2. watchdog_timer 를 끄지 않은 상태에서, 전역변수를 interrupt 로 바꾸면 일정 클럭 이후 초기화 된다.

 3. watchdog_timer 를 끄지 않은 상태에서 
    어떤 변수 A를 함수를 이용하여 값을 B로 바꾸고, interrupt function 으로 값을 바꾸면
    일정 클록 이후 다시 B로 리셋된다. (A로 리셋되는 것이 아니라, 바로 이전 값으로 reset 된다.)

    -> 아마도 바로 이전 상태를 저장하는 어떤 구조가 있는 것 같다.
*/

#include <msp430.h>
#define ADC_DELTA_TEN_TIME 1507
#define ADC_MIN 1081
#define ADC_MAX 4095

unsigned int temp = 4; // 초기값 A로 되어있다.

/* watchdog timer functions */

//void stop_watchdog_timer(void); // 이것을 주석처리하여 watchdog_timer 를 끌 수 있다.

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
    //stop_watchdog_timer(); // 이 코드가 없으면 while loop 에서 일정 시간이 흐르고 나면 전역변수가 초기화됨.
    init_right_switch();
    enable_interrupt_vector();

    change_temp(); // 일반함수를 통해 temp 를 A에서 B로 바꾼다.

    while(1){
        // 일정 클록이 지나면 인터럽트에 의해 C로 바뀌었던 B 가 다시 B로 리셋된다.
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
    temp = 94; // interrupt 에 의해 temp 를 C로 바꾼다.
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


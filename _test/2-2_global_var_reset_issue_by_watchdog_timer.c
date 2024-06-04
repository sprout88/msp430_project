#include <msp430.h>
#define ADC_DELTA_TEN_TIME 1507
#define ADC_MIN 1081
#define ADC_MAX 4095

unsigned int temp = 4;

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
    stop_watchdog_timer(); // 이 코드가 없으면 while loop 에서 일정 시간이 흐르고 나면 전역변수가 초기화됨.
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
    temp = 326;
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


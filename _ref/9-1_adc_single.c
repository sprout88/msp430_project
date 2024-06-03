#include <msp430.h>

unsigned int data = 0;

void stop_watchdog_timer(void);

void init_led1(void);
void init_led2(void);

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;

    P1DIR |= BIT0;
    P4DIR |= BIT7;

    
}

void init_led1(void){
    // LED1 DIR : P1.1
    P1DIR |= BIT0;   
    // LED1 OFF
    P1OUT &= ~BIT0;
}

void init_led2(void){
    // LED2 DIR : P4.7
    P4DIR |= BIT7;
    // LED2 OFF
    P4OUT &= ~BIT7; // LED2 OFF
}
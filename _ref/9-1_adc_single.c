#include <msp430.h>

unsigned int data = 0;

void stop_watchdog_timer(void);

void init_led1(void);
void init_led2(void);
void init_ADC(void);

void turn_on_led(int led_num);
void turn_off_led(int led_num);

void main(void)
{
    init_led1();
    init_led2();

    init_ADC();

    while(1){
        ADC12CTL0 |= ADC12SC;
        while(!(ADC12IFG & BIT0));
        data = ADC12MEM0;

        if(data > 3000)
        {
        }
    }
}

void init_led(){
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

void turn_on_led(int led_num){
    switch led_num{
        case(1):
            P1OUT |= BIT0; // LED1 ON
        case(2):
            P4OUT |= BIT7; // LED2 ON
    }
}
void turn_off_led(int led_num){
    switch led_num{
        case(1):
            P1OUT &= ~BIT0; // LED1 OFF
        case(2):
            P4OUT &= ~BIT7; // LED2 OFF
    }
}

void init_ADC(void){
    P6SEL |= BIT0; // ADC DIR

    /* ADC12 control register set */
    /* sample hold time : 16 adc clock cycles, ADC12 ON */ 
    ADC12CTL0 = ADC12SHT02 + ADC12ON;

    ADC12CTL1 = ADC12SHP; // sample hold : pulse mode
    ADC12MCTL0 = ADC12INCH_0; // input channel=A0
    ADC12CTL0 |= ADC12ENC; // ADC12 encoding=enable
}
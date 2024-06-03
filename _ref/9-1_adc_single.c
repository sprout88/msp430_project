/*
ADC 를 돌리면, LED 가 순차적으로 점등한다.
*/

#include <msp430.h>

unsigned int data = 0;

void stop_watchdog_timer(void);

void init_led(int led_num);
void turn_on_led(int led_num);
void turn_off_led(int led_num);

void init_ADC_single_mode(void);
void init_ADC_repeat_single_mode(void);
void ADC_single_read(unsigned int* p_data);

void main(void)
{
    init_led(1);
    init_led(2);

    init_ADC();

    while(1){

        ADC_single_read(&data);

        if(data > 3000)
        {
            turn_on_led(1);
            turn_on_led(2);
        }
        else if(data > 2000)
        {
            turn_off_led(1);
            turn_on_led(2);
        }
        else // data < 2000
        {
            turn_off_led(1);
            turn_off_led(2);
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
            break;
    }
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
#include <msp430.h>

unsigned int digits[10] = { 0xdb, 0x50, 0x1f, 0x5d, 0xd4, 0xcd, 0xcf, 0xd8, 0xdf, 0xdd };

unsigned int cnt = 0;
unsigned long usec = 0;
unsigned int data = 0;
unsigned long start_time = 0;
unsigned long end_time = 0;
unsigned long delta_time = 0;
unsigned long distance = 0;

unsigned int flag = 0;
unsigned int i = 0;

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

    // Digital Output - 7 segment
    P3DIR |= 0xffff;
    P3OUT &= 0x0000;
    P4DIR |= 0x000f;
    P4OUT &= ~BIT0;

    // Ultrasonic
    P2DIR |= BIT7;
    P2OUT &= ~BIT7;  // Trig off
    P1IE |= BIT4;    // Interrupt enabled
    P1IES &= ~BIT4;  // Rising edge
    P1IES |= BIT4;   // Falling edge
    P1IFG &= ~BIT4;  // Clear interrupt flag

    // Timer - Timer0
    TA0CCTL0 = CCIE;
    TA0CCR0 = 5000;  // 1ms
    TA0CTL = TASSEL_2 + MC_1 + TACLR; // SMCLK : 1MHz / Up mode to CCR0

    // Timer - Timer1
    TA1CCTL0 = CCIE;
    TA1CCR0 = 50;    // 50us
    TA1CTL = TASSEL_2 + MC_1 + TACLR; // SMCLK : 1MHz / Up mode to CCR0

    __bis_SR_register(GIE); // Enable global interrupts

    while (1) {
        if (flag == 0) {
            P2OUT |= BIT7;  // Trig on
            __delay_cycles(10); // 10us
            P2OUT &= ~BIT7; // Trig off
            flag = 1;
        }
    }
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void) {
    if (P1IFG & BIT4) {
        if ((P1IES & BIT4) == 0) { // if rising edge
            usec = 0;
            // TACCTL1 |= TACLR;
            P1IES |= BIT4; // falling edge
        } else if (P1IES & BIT4) { // if falling edge
            if (usec > 3 && usec < 500) { // 150us ~ 25ms
                data = usec * 50 / 58; // data = usec * 50 / 58;
            } else if (usec >= 760) {
                data = 9999;
            }
            usec = 0;
            P1IES &= ~BIT4; // rising edge
        }
        flag = 0;
        P1IFG &= ~BIT4; // IFG is cleared
    }
}

// Timer interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void) {
    cnt++;
    if (cnt > 3)
        cnt = 0;

    switch (cnt) {
        case 0:
            P3OUT = digits[data % 10];
            P4OUT = ~BIT0;
            break;
        case 1:
            P3OUT = digits[(data / 10) % 10];
            P4OUT = ~BIT1;
            break;
        case 2:
            P3OUT = digits[(data / 100) % 10];
            P4OUT = ~BIT2;
            break;
        case 3:
            P3OUT = digits[(data / 1000) % 10];
            P4OUT = ~BIT3;
            break;
    }
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void) {
    usec++;
    if (flag == 1 && usec > 1000) {
        flag = 0;
    }
}


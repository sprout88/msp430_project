#include <msp430.h>

unsigned int digits[10] = { 0xdb, 0x50, 0x1f, 0x5d, 0xd4, 0xcd, 0xcf, 0xd8, 0xdf, 0xdd};

unsigned int dynamic_segment_cnt = 0;
unsigned int key = 0;

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer

    /* 7 segment Digital Output */ 
    P3DIR |= 0xffff;
    P3OUT &= 0x0000;
    P4DIR |= 0x000f;
    P4OUT &= ~BIT0;
    /* END 7 segment Digital Output */

    /* keypad */ 

    // output
    P2DIR |= (BIT0 | BIT2 | BIT3);
    P2OUT |= (BIT0 | BIT2 | BIT3); // all high

    // input
    P6REN |= (BIT3 | BIT4 | BIT5 | BIT6);
    P6OUT |= (BIT3 | BIT4 | BIT5 | BIT6); // pull up
    
    /* END keypad */

    // Timer - Timer0
    TA0CCTL0 = CCIE;
    TA0CCR0 = 5000; //1000;
    TA0CTL = TASSEL_2 + MC_1 + TACLR; // SMCLK : 1Mhz / Up mode to CCRO

    __bis_SR_register(GIE);

    while(1)
    {
        // columns 1
        P2OUT &= ~BIT2;
        P2OUT |= (BIT0 | BIT3);

        if((P6IN & BIT3) == 0)
        {
            key = 1;
        }
        else if((P6IN & BIT6) == 0)
        {
            key = 4;
        }
        else if((P6IN & BIT5) == 0)
        {
            key = 7;
        }
        else if((P6IN & BIT4) == 0)
        {
            key = 10;
        }

        // columns 2
        P2OUT &= ~BIT0;
        P2OUT |= (BIT2 | BIT3);

        if((P6IN & BIT3) == 0)
        {
            key = 2;
        }
        else if((P6IN & BIT6) == 0)
        {
            key = 5;
        }
        else if((P6IN & BIT5) == 0)
        {
            key = 8;
        }
        else if((P6IN & BIT4) == 0)
        {
            key = 0;
        }

        // columns 3
        P2OUT &= ~BIT3;
        P2OUT |= (BIT0 | BIT2);

        if((P6IN & BIT3) == 0)
        {
            key = 3;
        }
        else if((P6IN & BIT6) == 0)
        {
            key = 6;
        }
        else if((P6IN & BIT5) == 0)
        {
            key = 9;
        }
        else if((P6IN & BIT4) == 0)
        {
            key = 11;
        }
    }
}

// Timer interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
    dynamic_segment_cnt++;
    if (dynamic_segment_cnt > 3)
        dynamic_segment_cnt = 0;

    switch (dynamic_segment_cnt)
    {
    case 0:
        P3OUT = digits[key%10];
        P4OUT = ~BIT0;
        break;
    case 1:
        P3OUT = digits[key/10%10];
        P4OUT = ~BIT1;
        break;
    case 2:
        P3OUT = digits[key/100%10];
        P4OUT = ~BIT2;
        break;
    case 3:
        P3OUT = digits[key/1000%10];
        P4OUT = ~BIT3;
        break;
    }
}

#include <msp430.h>

unsigned int digits[10] = { 0xdb, 0x50, 0x1f, 0x5d, 0xd4, 0xcd, 0xcf, 0xd8, 0xdf, 0xdd };

unsigned int i = 0;
unsigned int cnt = 0;
unsigned int key = 0;
unsigned int data[4] = {0, };
unsigned int menu = 0;
unsigned int pwm_data = 0;

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD; // stop watchdog. timer
    // Digital Output - 7 segment
    P3DIR |= 0xffff;
    P3OUT &= 0x0000;
    P4DIR |= 0x000f;
    P4OUT &= ~BIT0;

    // Motor set
    P2DIR |= (BIT0 | BIT2); //left DIR : p2.0, right DIR : p2.2
    P2OUT &= ~(BIT0 | BIT2); //low

    // PWM set
    P2DIR |= (BIT5 | BIT4);
    P2SEL |= (BIT5 | BIT4);

    TA2CCR0 = 1000; //PWM, Top in Up mode
    TA2CCTL2 = OUTMOD_6; //PWM toggle/set
    TA2CCR2 = 0; //left speed
    TA2CCTL1 = OUTMOD_6; //PWM toggle/set
    TA2CCR1 = 0; //right speed
    TA2CTL = TASSEL_2 + MC_1; // SMCLK : 1Mhz / Up mode to CCRO

    //// keypad
    // output
    P2DIR |= (BIT0 | BIT2 | BIT3);
    P2OUT |= (BIT0 | BIT2 | BIT3); // all high

    // input
    P6REN |= (BIT3 | BIT4 | BIT5 | BIT6);
    P6OUT |= (BIT3 | BIT4 | BIT5 | BIT6); // pull up

    // Timer - Timer0
    TA0CCTL0 = CCIE;
    TA0CCR0 = 5000; //1000
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
        else if((P6IN & BIT4) == 0) // *
        {
            while((P6IN & BIT4) == 0)
                key = 0; //key = 10;
                menu++;
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
        else if((P6IN & BIT4) == 0) // #
        {
            //key = 11;
            menu = 0;
            TA2CCR2 = 0;
            TA2CCR1 = 0;
            for(i=0; i<4 ; i++)
            {
                data [i] = 0;
            }
        }

        // save key_number
        if(menu == 0)
        {
            data [3] = key;
        }
        else if(menu == 1)
        {
            data [2] = key;
        }
        else if(menu == 2)
        {
            data[1] = key;
        }
        else if(menu == 3)
        {
            data[0] = key;
        }
        else if(menu == 4)
        {
            menu = 0;
            pwm_data = data[3]*1000 + data[2]*100 + data[1]*10 + data[0];

            if(pwm_data <= 1000)
            {
                TA2CCR2 = pwm_data;
                TA2CCR1 = 0;
            }
            else
            {
                TA2CCR2 = 0;
                TA2CCR1 = 0;
                for (i=0 ; i<4 ; i++)
                {
                    data[i] = 0;
                }
            }
        }
    }
}

// Timer interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
    cnt++;
    if(cnt > 3)
        cnt = 0;

    switch (cnt)
    {
    case 0:
        P3OUT = digits[data[0]]; // 가장 낮은 자리의 데이터
        P4OUT = ~BIT0;
        break;
    case 1:
        P3OUT = digits[data[1]]; // 두 번째로 낮은 자리의 데이터
        P4OUT = ~BIT1;
        break;
    case 2:
        P3OUT = digits[data[2]]; // 세 번째로 낮은 자리의 데이터
        P4OUT = ~BIT2;
        break;
    case 3:
        P3OUT = digits[data[3]]; // 가장 높은 자리의 데이터
        P4OUT = ~BIT3;
        break;
    }
}

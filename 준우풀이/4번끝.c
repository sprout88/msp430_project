#include <msp430.h>

int encoder_count = 0; // 엔코더 회전 값
unsigned int d_click = 0;
unsigned int click_count = 0;
unsigned int state = 0;
unsigned int key = 0;
unsigned int key_seg[3] = { 0, }; // 7-segment 에 나타나는 숫자 key_seg[ ] 저장
unsigned int key_sum = 0;
unsigned int key_seg_num = 0; // 키패드에서 *을 누를때마다 증가하는 변수
unsigned int i = 0;
unsigned int seg_count = 0;
unsigned int love_count = 0;
unsigned int lcd_count = 0;
unsigned int pwm_seg[4] = { 0, };
unsigned int pwm_count = 0;
int seg[10] = {
BIT0 | BIT1 | BIT3 | BIT4 | BIT6 | BIT7, // 0
        BIT4 | BIT6,                             // 1
        BIT0 | BIT1 | BIT2 | BIT3 | BIT4,        // 2
        BIT0 | BIT2 | BIT3 | BIT4 | BIT6,        // 3
        BIT2 | BIT4 | BIT6 | BIT7,               // 4
        BIT0 | BIT2 | BIT3 | BIT6 | BIT7,        // 5
        BIT0 | BIT1 | BIT2 | BIT3 | BIT6 | BIT7, // 6
        BIT3 | BIT4 | BIT6,                      // 7
        BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT6 | BIT7, // 8
        BIT0 | BIT2 | BIT3 | BIT4 | BIT6         // 9
};

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    // 스위치
    P1OUT |= BIT1;
    P1REN |= BIT1;
    P1IE |= BIT1;
    P1IES |= BIT1;
    P1IFG &= ~BIT1;

//    P2OUT |= BIT1;
//    P2REN |= BIT1;
//    P2IE |= BIT1;
//    P2IES |= BIT1;
//    P2IFG &= ~BIT1;

    // Digital Output - 7 segment
    P3DIR |= 0xffff;
    P3OUT &= 0x0000;
    P4DIR |= 0x000f;
    P4OUT &= ~BIT0;

    // 엔코더 Interrupt set
    P1IE |= BIT3;           //Interrupt enabled
    P1IES |= BIT3;          //Interrupt edge(Falling Edge)
    P1IFG &= ~BIT3;         //Interrupt flag
    P1IE |= BIT2;           //Interrupt enabled
    P1IES |= BIT2;          //Interrupt edge(Falling Edge)
    P1IFG &= ~BIT2;         //Interrupt flag

    // 키패드
    // output
    P2DIR |= (BIT0 | BIT2 | BIT3);
    P2OUT |= (BIT0 | BIT2 | BIT3); // all high

    // input
    P6REN |= (BIT3 | BIT4 | BIT5 | BIT6);
    P6OUT |= (BIT3 | BIT4 | BIT5 | BIT6); // pull up

    // 모터
    P2DIR |= (BIT0 | BIT2);     // left DIR: p2.0, right DIR: p2.2
    P2OUT &= ~(BIT0 | BIT2);   // low

    // PWM
    P2DIR |= (BIT5 | BIT4);
    P2SEL |= (BIT5 | BIT4);

    TA2CCR0 = 1000;             // PWM, Top in Up mode
    TA2CCTL2 = OUTMOD_6;        // PWM toggle/set
    TA2CCR2 = 0;                // left speed
    TA2CCTL1 = OUTMOD_6;        // PWM toggle/set
    TA2CCR1 = 0;                // right speed
    TA2CTL = TASSEL_2 + MC_1;   // SMCLK: 1Mhz / Up mode to CCR0

    // Timer0
    TA0CCTL0 = CCIE;
    TA0CCR0 = 5000; // 5ms // 5ms *
    TA0CTL = TASSEL_2 + MC_1 + TACLR; // TASSEL_2: SMCLK: 1Mhz / Up mode to CCR0

    // Timer1
    TA1CCTL0 = CCIE;
    TA1CCR0 = 5000;
    TA1CTL = TASSEL_2 + MC_1 + TACLR; //TASSEL_2 // TASSEL_1: ACLK: 1Mhz / Up mode to CCR0

    __bis_SR_register(GIE);

    while (1)
    {
        if (state == 0)
        {
            P4OUT |= (BIT0 | BIT1 | BIT2 | BIT3);
            TA2CCR1 = 0;
            TA2CCR2 = 0;
        }
        if (state == 1)
        {
            if (encoder_count / 100 == 11)
            {
                // columns 1
                P2OUT &= ~BIT2;
                P2OUT |= (BIT0 | BIT3);

                if ((P6IN & BIT3) == 0)
                {
                    while (P6IN & BIT3)
                    {
                    } //채터링 방지
                    key = 1;
                    key_seg_num = key_seg_num + 1;
                }
                else if ((P6IN & BIT6) == 0)
                {
                    while (P6IN & BIT6)
                    {
                    }
                    key = 4;
                    key_seg_num = key_seg_num + 1;
                }
                else if ((P6IN & BIT5) == 0)
                {
                    while (P6IN & BIT5)
                    {
                    }
                    key = 7;
                    key_seg_num = key_seg_num + 1;
                }
                else if ((P6IN & BIT4) == 0)
                {
                    // * 아무일 안 하기.
                }
                // columns 2
                P2OUT &= ~BIT0;
                P2OUT |= (BIT2 | BIT3);

                if ((P6IN & BIT3) == 0)
                {
                    while (P6IN & BIT3)
                    {
                    }
                    key = 2;
                    key_seg_num = key_seg_num + 1;
                }
                else if ((P6IN & BIT6) == 0)
                {
                    while (P6IN & BIT6)
                    {
                    }
                    key = 5;
                    key_seg_num = key_seg_num + 1;
                }
                else if ((P6IN & BIT5) == 0)
                {
                    while (P6IN & BIT5)
                    {
                    }
                    key = 8;
                    key_seg_num = key_seg_num + 1;
                }
                else if ((P6IN & BIT4) == 0)
                {
                    while (P6IN & BIT4)
                    {
                    }
                    key = 0;
                    key_seg_num = key_seg_num + 1;
                }

                // columns 3
                P2OUT &= ~BIT3;
                P2OUT |= (BIT0 | BIT2);

                if ((P6IN & BIT3) == 0)
                {
                    while (P6IN & BIT3)
                    {
                    }
                    key = 3;
                    key_seg_num = key_seg_num + 1;
                }
                else if ((P6IN & BIT6) == 0)
                {
                    while (P6IN & BIT6)
                    {
                    }
                    key = 6;
                    key_seg_num = key_seg_num + 1;
                }
                else if ((P6IN & BIT5) == 0)
                {
                    while (P6IN & BIT3)
                    {
                    }
                    key = 9;
                    key_seg_num = key_seg_num + 1;
                }
                else if ((P6IN & BIT4) == 0)
                { // #
                    key_seg_num = 0;
                    for (i = 0; i < 3; i++)
                    {
                        key_seg[i] = 0;
                    }
                }
                // Update 7-segment display only if a key was pressed
                if (key_seg_num == 0)
                    P4OUT |= (BIT0 | BIT1 | BIT2);
                if (key_seg_num == 1)
                {
                    key_seg[2] = key;
                    P4OUT |= (BIT0 | BIT1);
                    __delay_cycles(100000); //너무 빨리 인식해서 [0,0,key]가 아닌 [key,key,key]가 되어버림
                }
                if (key_seg_num == 2)
                {
                    key_seg[1] = key;
                    P4OUT |= BIT0;
                    __delay_cycles(100000);

                }
                if (key_seg_num == 3)
                {
                    key_seg[0] = key;
                    __delay_cycles(100000);

                }
                key_sum = key_seg[2] * 100 + key_seg[1] * 10 + key_seg[0];
                if (key_seg_num > 4)
                    key = 0;
            }
        }
        if (state == 2)
        {
            key = 0;

        }
        if (state == 3)
        {
            if ((pwm_count % 6) == 0)
            {
                pwm_seg[3] = 0;
                pwm_seg[2] = 4;
                pwm_seg[1] = 8;
                pwm_seg[0] = 6;
                TA2CCR1 = 486;
                TA2CCR2 = 0;
            }
            else if ((pwm_count % 6) == 1)
            {
                pwm_seg[3] = 0;
                pwm_seg[2] = 8;
                pwm_seg[1] = 6;
                pwm_seg[0] = 4;
                TA2CCR1 = 0;
                TA2CCR2 = 864;
            }
            else if ((pwm_count % 6) == 2)
            {
                pwm_seg[3] = 0;
                pwm_seg[2] = 6;
                pwm_seg[1] = 4;
                pwm_seg[0] = 8;
                TA2CCR1 = 648;
                TA2CCR2 = 0;
            }
            else if ((pwm_count % 6) == 3)
            {
                pwm_seg[3] = 0;
                pwm_seg[2] = 4;
                pwm_seg[1] = 8;
                pwm_seg[0] = 6;
                TA2CCR1 = 0;
                TA2CCR2 = 486;
            }
            else if ((pwm_count % 6) == 4)
            {
                pwm_seg[3] = 0;
                pwm_seg[2] = 8;
                pwm_seg[1] = 6;
                pwm_seg[0] = 4;
                TA2CCR1 = 864;
                TA2CCR2 = 0;
            }
            else if ((pwm_count % 6) == 5)
            {
                pwm_seg[3] = 0;
                pwm_seg[2] = 6;
                pwm_seg[1] = 4;
                pwm_seg[0] = 8;
                TA2CCR1 = 0;
                TA2CCR2 = 648;
            }
        }
    }
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    if ((P1IN & BIT1) == 0)
    {
        if (state == 0) //초기 상태에서 P1.1 스위치 누름
        {
            while ((P1IN & BIT1) == 0)
            {
            } // 채터링 잡는 코드
            if (d_click == 0)
            {
                d_click = 1; // 첫 번째 클릭
                click_count = 0;
            }
            else if (d_click == 1 && click_count < 60) // 두 번째 클릭 (더블 클릭) 5ms * 60 = 300ms = 0.3s
                d_click = 2; // 두 번째 클릭 (더블 클릭)
            else
            { // 클릭 간격이 너무 길면 첫 번째 클릭으로 다시 설정
                d_click = 1;
                click_count = 0;
            }
        }
        else  if (state == 1)
        {
            while ((P1IN & BIT1) == 0){} // 채터링 잡는 코드
            if (key_sum == 486)
            {
                TA0CTL = TASSEL_2 + MC_1; // 타이머 A0 시작
                TA1CTL = TASSEL_2 + MC_1; // 타이머 A1 시작
                state = 2;
            }
            else
                state = 1;
        }
        else if (state == 2) //상태2에서 P1.1 스위치 누름
        {
            while ((P1IN & BIT1) == 0){}
            state = 3;
        }
        else if (state == 3)
        {
            while ((P1IN & BIT1) == 0){}
            __delay_cycles(1000);
            state = 0;
        }
    }

    if (state == 1)
    {
        if (P1IFG & BIT2)
        {
            if ((P1IN & BIT3) != 0)
            {
                encoder_count--; // 정방향
            }
            else
            {
                encoder_count++;
            }
        }
        if (P1IFG & BIT3)
        {
            if ((P1IN & BIT2) == 0)
            {
                encoder_count--;
            }
            else
            {
                encoder_count++;
            }
        }
        if (encoder_count > 1200)
            encoder_count = 0;
        else if (encoder_count < 0)
            encoder_count = 1200;
    }
    else if (state == 3)
    {
        if (P1IFG & BIT2)
        {
            if ((P1IN & BIT3) != 0)
            {
                encoder_count++; //여기선 그냥 카운트 +만 하자.
            }
            else
            {
                encoder_count++;
            }
        }
        if (P1IFG & BIT3)
        {
            if ((P1IN & BIT2) == 0)
            {
                encoder_count++;
            }
            else
            {
                encoder_count++;
            }
        }
        if (encoder_count > 6700) //10바퀴에 약 6700
            encoder_count = 0;
        //pwm 포트1 인터럽트로 사용
        if (encoder_count >= 6700) //정확하게 6670인듯??
        {
            pwm_count++;
            TA2CCR1 = 0;
            TA2CCR2 = 0;
            if (pwm_count > 6)
                pwm_count = 0;
        }
    }
P1IFG &= ~BIT1;
P1IFG &= ~BIT2;
P1IFG &= ~BIT3;
}

#pragma vector=PORT2_VECTOR //안 씀
__interrupt void Port_2(void){}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
if (state == 0)
{
    if (d_click == 1)
        click_count++;
    if (click_count >= 60) // 5ms * 60 = 300ms = 0.3s
    { // 첫 번째 클릭 후 0.3초 경과하면 단일 클릭으로 간주
        d_click = 0;
        click_count = 0;
    }
    else if (d_click == 2)
    { // 더블 클릭 처리
        state = 1;
        P3OUT = BIT7;
        P4OUT &= ~BIT3;
        P4OUT |= (BIT0 | BIT1 | BIT2);
        d_click = 0; // 더블 클릭 상태 초기화
        click_count = 0;
    }
}

if (state == 1)
{
    switch (encoder_count / 100)
    { // 0~11까지 존재
    case 0:
        P3OUT = BIT7;
        P4OUT &= ~BIT3;
        P4OUT |= (BIT0 | BIT1 | BIT2);
        break;
    case 1:
        P3OUT = BIT3;
        P4OUT &= ~BIT3;
        P4OUT |= (BIT0 | BIT1 | BIT2);
        break;
    case 2:
        P3OUT = BIT3;
        P4OUT &= ~BIT2;
        P4OUT |= (BIT0 | BIT1 | BIT3);
        break;
    case 3:
        P3OUT = BIT3;
        P4OUT &= ~BIT1;
        P4OUT |= (BIT0 | BIT2 | BIT3);
        break;
    case 4:
        P3OUT = BIT3;
        P4OUT &= ~BIT0;
        P4OUT |= (BIT1 | BIT2 | BIT3);
        break;
    case 5:
        P3OUT = BIT4;
        P4OUT &= ~BIT0;
        P4OUT |= (BIT1 | BIT2 | BIT3);
        break;
    case 6:
        P3OUT = BIT6;
        P4OUT &= ~BIT0;
        P4OUT |= (BIT1 | BIT2 | BIT3);
        break;
    case 7:
        P3OUT = BIT0;
        P4OUT &= ~BIT0;
        P4OUT |= (BIT1 | BIT2 | BIT3);
        break;
    case 8:
        P3OUT = BIT0;
        P4OUT &= ~BIT1;
        P4OUT |= (BIT0 | BIT2 | BIT3);
        break;
    case 9:
        P3OUT = BIT0;
        P4OUT &= ~BIT2;
        P4OUT |= (BIT0 | BIT1 | BIT3);
        break;
    case 10:
        P3OUT = BIT0;
        P4OUT &= ~BIT3;
        P4OUT |= (BIT0 | BIT1 | BIT2);
        break;
    case 11:
        P3OUT = BIT1;
        P4OUT &= ~BIT3;
        P4OUT |= (BIT0 | BIT1 | BIT2);
        break;
    }
}
if (state == 1 && key_seg_num == 1) //key_seg[2]가 입력되면 P4.2 P4.3출력
{ //세그먼트 다이나믹 구동
    seg_count++;
    if (seg_count > 1)
        seg_count = 0;
    switch (seg_count)
    {
    case 0:
        P3OUT = BIT1;
        P4OUT = ~BIT3;
        P4OUT |= (BIT0 | BIT1 | BIT1);
        break;
    case 1:
        P3OUT = seg[key_seg[2]];
        P4OUT = ~BIT2;
        P4OUT |= (BIT0 | BIT1 | BIT3);
        break;
    case 2:
        P3OUT = seg[key_seg[1]];
        P4OUT = ~BIT1;
        P4OUT |= (BIT0 | BIT2 | BIT3);
        break;
    case 3:
        P3OUT = seg[key_seg[0]];
        P4OUT = ~BIT0;
        P4OUT |= (BIT1 | BIT2 | BIT3);
        break;
    }
}
if (state == 1 && key_seg_num == 2) //key_seg[1]가 입력되면 P4.1 P4.2 P4.3출력
{ //세그먼트 다이나믹 구동
    seg_count++;
    if (seg_count > 2)
        seg_count = 0;
    switch (seg_count)
    {
    case 0:
        P3OUT = BIT1;
        P4OUT = ~BIT3;
        P4OUT |= (BIT0 | BIT1 | BIT1);
        break;
    case 1:
        P3OUT = seg[key_seg[2]];
        P4OUT = ~BIT2;
        P4OUT |= (BIT0 | BIT1 | BIT3);
        break;
    case 2:
        P3OUT = seg[key_seg[1]];
        P4OUT = ~BIT1;
        P4OUT |= (BIT0 | BIT2 | BIT3);
        break;
    case 3:
        P3OUT = seg[key_seg[0]];
        P4OUT = ~BIT0;
        P4OUT |= (BIT1 | BIT2 | BIT3);
        break;
    }
}
if (state == 1 && key_seg_num == 3) //key_seg[0]가 입력되면 P4.0 P4.1 P4.2 P4.3출력
{ //세그먼트 다이나믹 구동
    seg_count++;
    if (seg_count > 3)
        seg_count = 0;
    switch (seg_count)
    {
    case 0:
        P3OUT = BIT1;
        P4OUT = ~BIT3;
        P4OUT |= (BIT0 | BIT1 | BIT2);
        break;
    case 1:
        P3OUT = seg[key_seg[2]];
        P4OUT = ~BIT2;
        P4OUT |= (BIT0 | BIT1 | BIT3);
        break;
    case 2:
        P3OUT = seg[key_seg[1]];
        P4OUT = ~BIT1;
        P4OUT |= (BIT0 | BIT2 | BIT3);
        break;
    case 3:
        P3OUT = seg[key_seg[0]];
        P4OUT = ~BIT0;
        P4OUT |= (BIT1 | BIT2 | BIT3);
        break;
    }
}
if (state == 2)
{ //love출력
    love_count++;
    if (love_count > 3)
        love_count = 0;
    switch (love_count)
    {
    case 0:
        P3OUT = (BIT0 | BIT1 | BIT7);
        P4OUT = ~BIT3;
        P4OUT |= (BIT0 | BIT1 | BIT2);
        break;
    case 1:
        P3OUT = (BIT0 | BIT1 | BIT3 | BIT4 | BIT6 | BIT7);
        P4OUT = ~BIT2;
        P4OUT |= (BIT0 | BIT1 | BIT3);
        break;
    case 2:
        P3OUT = (BIT7 | BIT1 | BIT0 | BIT6 | BIT4);
        P4OUT = ~BIT1;
        P4OUT |= (BIT0 | BIT2 | BIT3);
        break;
    case 3:
        P3OUT = (BIT0 | BIT1 | BIT2 | BIT3 | BIT7);
        P4OUT = ~BIT0;
        P4OUT |= (BIT1 | BIT2 | BIT3);
        break;
    }
    seg_count++;
    if (seg_count > 50) // 5ms * 100 = 500ms = 0.5s
    {
        lcd_count++;
        if (lcd_count > 1)
            lcd_count = 0;
        if (lcd_count == 1)
        {
            TA0CTL = TASSEL_2 + MC_0; // 타이머 A0 정지
            TA1CTL = TASSEL_2 + MC_1; // 타이머 A1 시작
        }
        seg_count = 0;
    }
}
if (state == 3)
{ //세그먼트 다이나믹 구동
    seg_count++;
    if (seg_count > 3)
        seg_count = 0;
    switch (seg_count)
    {
    case 0:
        P3OUT = seg[pwm_seg[3]];
        P4OUT = ~BIT3;
        P4OUT |= (BIT0 | BIT1 | BIT1);
        break;
    case 1:
        P3OUT = seg[pwm_seg[2]];
        P4OUT = ~BIT2;
        P4OUT |= (BIT0 | BIT1 | BIT3);
        break;
    case 2:
        P3OUT = seg[pwm_seg[1]];
        P4OUT = ~BIT1;
        P4OUT |= (BIT0 | BIT2 | BIT3);
        break;
    case 3:
        P3OUT = seg[pwm_seg[0]];
        P4OUT = ~BIT0;
        P4OUT |= (BIT1 | BIT2 | BIT3);
        break;
    }
}
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void)
{
    if (state == 2)
    {
        P4OUT |= (BIT0 | BIT1 | BIT2 | BIT3);
        seg_count++;
        if (seg_count > 50) // 5ms * 100 = 500ms = 0.5s
        {
            lcd_count++;
            if (lcd_count > 1)
                lcd_count = 0;
            if (lcd_count == 1)
            {
                TA0CTL = TASSEL_2 + MC_1; // 타이머 A0 시작
                TA1CTL = TASSEL_2 + MC_0; // 타이머 A1 정지
            }
            seg_count = 0;
        }
    }
}

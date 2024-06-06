#include <msp430.h>
unsigned int a = 0;
unsigned int encoder_count = 0;
unsigned cnt = 0;
unsigned data = 0;        // ADC 값 받아오는 데이터 값
unsigned menu = 0;               // P1.1 스위치를 누름에 따라 증가하는 값
unsigned int adc_data = 0; // ADC 값 매핑
unsigned int adc_value[4] = {0, }; //세그먼트에 넣어주게 만듦
unsigned state = 0;               // 전체적인 동작의 상태 설정
unsigned pw[4] = {0, };          // encoder_count를 이용한 비밀번호 설정에 이용되는 변수
unsigned key = 0;
unsigned keymenu = 0;            // 키패드에서 *을 누를때마다 증가하는 변수
unsigned i = 0;
unsigned int led_cnt = 0; // led 타이머에 쓸 카운트
unsigned int led_cnt_req = 0; // adc값에 따라 켜지는 시간을 정하는 변수
unsigned int cho_stop = 0; //초음파 센서에 사용하는 변수
unsigned int cho_data = 0;
unsigned long cho_timer = 0;
unsigned int motor_for = 0; // 모터 속도를 저장할 변수 추가
unsigned int motor_rev = 0; // 모터 속도를 저장할 변수 추가
unsigned int pre_motor_for = 0; // 이전 모터 속도 변수
unsigned int pre_motor_rev = 0; // 이전 모터 속도 변수

int seg[10] ={
              BIT0 | BIT1 | BIT3 | BIT4 | BIT6 | BIT7,    // P3.0, 3.1, 3.3, 3.4, 3.6, 3.7이 1이 되면 0을 출력
              BIT4 | BIT6,                                // 1
              BIT0 | BIT1 | BIT2 | BIT3 | BIT4,           // 2
              BIT0 | BIT2 | BIT3 | BIT4 | BIT6,           // 3
              BIT2 | BIT4 | BIT6 | BIT7,                  // 4
              BIT0 | BIT2 | BIT3 | BIT6 | BIT7,           // 5
              BIT0 | BIT1 | BIT2 | BIT3 | BIT6 | BIT7,    // 6
              BIT3 | BIT4 | BIT6,                         // 7
              BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT6 | BIT7, // 8
              BIT0 | BIT2 | BIT3 | BIT4 | BIT6 | BIT7     // 9
              };

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    // Digital Output - 7 segment
    P3DIR |= 0xffff;
    P3OUT &= 0x0000;
    P4DIR |= 0x000f;
    P4OUT &= ~BIT0;

    // 엔코더 Interrupt set
    P1IE |= BIT3;           //Interrupt enabled
    P1IES |= BIT3;          //Interrupt edge(Falling Edge)
    P1IFG &= ~BIT3;         //Interrupt cho_stop
    P1IE |= BIT2;           //Interrupt enabled
    P1IES |= BIT2;          //Interrupt edge(Falling Edge)
    P1IFG &= ~BIT2;         //Interrupt cho_stop

    // LED 설정

    P1DIR |= BIT0; // P1.0을 출력으로 설정
    P4DIR |= BIT7;

    P1OUT &= ~BIT0; //left LED off
    P4OUT &= ~BIT7; //right LED off

    // 스위치
    P1OUT |= BIT1;
    P1REN |= BIT1;
    P1IE |= BIT1;
    P1IES |= BIT1;
    P1IFG &= ~BIT1;

    P2OUT |= BIT1;
    P2REN |= BIT1;
    P2IE |= BIT1;
    P2IES |= BIT1;
    P2IFG &= ~BIT1;

    // 모터
    P2DIR |= (BIT0 | BIT2);     //left DIR : p2.0, right DIR : p2.2
    P2OUT &= ~ (BIT0 | BIT2);   //low

    // PWM
    P2DIR |= (BIT5 | BIT4);
    P2SEL |= (BIT5 | BIT4);

    TA2CCR0 = 1000;             //PWM, Top in Up mode
    TA2CCTL2 = OUTMOD_6;        //PWM toggle/set
    TA2CCR2 = 0;                //left speed
    TA2CCTL1 = OUTMOD_6;        //PWM toggle/set
    TA2CCR1 = 0;                //right speed
    TA2CTL = TASSEL_2 + MC_1;   // SMCLK : 1Mhz / Up mode to CCR0

    // ADC 가변저항 반복 싱글 채널
    P6SEL |= BIT0;
    ADC12CTL0 = ADC12SHT02 + ADC12MSC + ADC12ON; //샘플링 주기 설정 및 ADC ON
    ADC12CTL1 = ADC12SHP + ADC12CONSEQ_2; //샘플링 주기를 설정하기 위한 타이머 설정
    ADC12MCTL0 = ADC12INCH_0; //A0 채널 선택
    ADC12CTL0 |= ADC12ENC; // ADC 활성화
    ADC12CTL0 |= ADC12SC; //데이터 받아오기

    // 초음파 센서
    P2DIR |= BIT7; // trig output으로 설정
    P2OUT &= ~BIT7; // P2.7에 0을 넣겠다.
    P1IE |= BIT4; //P1.4 인터럽트 연결
    P1IES &= ~BIT4; // rising edge로 선택 P1IES |= BIT4;이면 falling edge임
    P1IFG &= ~BIT4; // 인터럽트 플래그 내려줌

    // 키패드
    // output
    P2DIR |= (BIT0|BIT2|BIT3);
    P2OUT |= (BIT0|BIT2|BIT3); // all high

    //input
    P6REN |= (BIT3|BIT4|BIT5|BIT6);
    P6OUT |= (BIT3|BIT4|BIT5|BIT6); // pull up

    // Timer0
    TA0CCTL0 = CCIE;
    TA0CCR0 = 1000; // 1ms
    TA0CTL = TASSEL_2 + MC_1 + TACLR; // SMCLK : 1Mhz / Up mode to CCR0

    // Timer1
    TA1CCTL0 = CCIE;
    TA1CCR0 = 50;
    TA1CTL = TASSEL_2 + MC_1 + TACLR; // TASSEL_1: ACLK : 1Mhz / Up mode to CCR0

    __bis_SR_register(GIE);

    while(1)
    {
        if(state == 0) // state0 초기상태
        {
            P4OUT |= BIT0|BIT1|BIT2|BIT3;
        }
        else if(state == 1) // 스위치1번 누른 상태. ADC 값 mapping
        {
            data = ADC12MEM0;
            adc_data = (ADC12MEM0-1000)/309.5 * 2;
            adc_value[3] = adc_data/1000%10;
            adc_value[2] = adc_data/100%10;
            adc_value[1] = adc_data/10%10;
            adc_value[0] = adc_data%10;
        }
        else if(state == 2) // 스위치 2번 누른 상태, LED on
        {
            led_cnt_req = adc_data * 100;
        }
        else if(state == 3) //스위치 3번 누른 상태, LED off, 7초에 맞춰 PWM 최대로
        {
            TA0CCR0 = 0; //Timer0 인터럽트 끄기
            TA1CCR0 = 0;
            P4OUT |= BIT0|BIT1|BIT2|BIT3;
            // columns 1
            P2OUT &= ~BIT2;
            P2OUT |= (BIT0 | BIT3);
            if((P6IN & BIT4) == 0) // *
            {
                if(TA2CCR2 < 300) //PWM300이하는 생략
                {
                    TA2CCR2 = 300;
                    __delay_cycles(10000);
                }

                if(TA2CCR2 < 1000) // 30%이하 생략:300 + 1초에 100올라가게 설정:7초에 700 = 1000
                {
                    TA2CCR2++;
                    __delay_cycles(10000);
                }
                else if(TA2CCR1>0) //다른 방향인 경우 감속시킨다.
                {
                    TA2CCR1--;
                    __delay_cycles(10000);
                }
                else if(TA2CCR1 == 0)
                    TA2CCR2 ++;
            }
            // columns 3
            P2OUT &= ~BIT3;
            P2OUT |= (BIT0 | BIT2);
            if((P6IN & BIT4) == 0) // #
            {
                if(TA2CCR1 < 300) //PWM300이하는 생략
                {
                    TA2CCR1 = 300;
                    __delay_cycles(10000);
                }

                if(TA2CCR1 < 1000) // 30%이하 생략:300 + 1초에 100올라가게 설정:7초에 700 = 1000
                {
                    TA2CCR1++;
                    __delay_cycles(10000);
                }
                else if(TA2CCR2>0) //다른 방향인 경우 감속시킨다.
                {
                    TA2CCR2--;
                    __delay_cycles(10000);
                }
                else if(TA2CCR2 == 0)
                    TA2CCR1 ++;
            }
            motor_for = TA2CCR1; // 모터 속도를 저장
            motor_rev = TA2CCR2;
        }
        else if(state == 4)
        {
            adc_value[3] = cho_data/1000%10;
            adc_value[2] = cho_data/100%10;
            adc_value[1] = cho_data/10%10;
            adc_value[0] = cho_data%10;
            if(cho_stop == 0)
            {
                P2OUT |= BIT7;
                __delay_cycles(10);
                P2OUT &= ~BIT7;
                cho_stop = 1;
            }
            if(cho_data < 10) //10cm 이하 모터 정지
            {
                TA2CCR1 = 0;
                TA2CCR2 = 0;
            }
            else
            {
                // state 3에서 설정한 모터 속도 값을 사용
                TA2CCR1 = motor_for;
                TA2CCR2 = motor_rev;
            }
        }
        else if(state == 5)
        {
            P4OUT |= BIT0|BIT1|BIT2|BIT3;

        }
    }
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    if(state == 4)
    {
        // 초음파 센서에 대한 인터럽트 처리
        if((P1IFG & BIT4) != 0)
        {
            if((P1IES & BIT4) == 0) // rising edge
            {
                cho_timer = 0; // 타이머 초기화
                P1IES |= BIT4; // falling edge로 설정
            }
            else if((P1IES & BIT4) != 0) // falling edge
            {
                // 타이머 값에 따라 거리 계산
                if(cho_timer > 3 && cho_timer < 500)
                    cho_data = cho_timer * 50 / 58;
                else if(cho_timer >= 760)
                    cho_data = 9999;
                else
                    cho_data = 0;
                P1IES &= ~BIT4; // rising edge로 재설정
            }
            P1IFG &= ~BIT4; // 인터럽트 플래그 클리어
        }
    }

    else if(state == 5)
    {//엔코더에 대한 인터럽트
        if(P1IFG & BIT2)
            {
                if((P1IN & BIT3) != 0)
                {
                    encoder_count--; //CW(-로 카운트)
                }
                else
                {
                    encoder_count++; //CCW(+로 카운트)
                }
            }

            if(P1IFG & BIT3)
            {
                if((P1IN & BIT2) == 0)
                {
                    encoder_count--;
                }
                else
                {
                    encoder_count++;
                }
            }
    }
    P1IFG &= ~BIT2;         //IFG is cleared
    P1IFG &= ~BIT3;         //IFG is cleared
    P1IFG &= ~BIT4;         //IFG is cleared
}


#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
    if((P2IN & BIT1) == 0)
    {
        while((P2IN & BIT1) == 0){}    //채터링 잡는 코드

        if(state == 0)   // 초기 상태에서 스위치를 누르면 state1로 감
            state = 1;

        else if(state == 1)  // state1에서 스위치를 누르면 state2로 간다. 그리고 두 LED를 교차로 toggle시키기 위해 한 쪽 led를 끈다.
        {
            state = 2;
            P1OUT |= BIT0;
            P4OUT &= ~BIT7;
        }
        else if(state == 2) // state2에서 스위치를 누르면 LED를 모두 끄고, state2로 감.
        {
            state = 3;
            P1OUT &= ~BIT0;
            P4OUT &= ~BIT7;
        }
        else if(state == 3) //state3에서 스위치를 누르며 pwm 값을 저장하고, state4로 간다.
        {
            TA0CCR0 = 1000; //Timer0 인터럽트 다시 켜기
            TA1CCR0 = 50;
            state = 4;
        }
        else if(state == 4)
            state = 5;

        else if(state == 5)
            state = 0;

    }
    P2IFG &= ~BIT1;
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
    if(state == 0)
        cnt = 4;
    if(state == 1){
        cnt++;
        if(cnt > 1)
            cnt = 0;
        switch (cnt)
        {
        case 0:
            P3OUT = seg[adc_value[0]]; // 가장 낮은 자리의 데이터
            P4OUT &= ~BIT0;
            P4OUT |= (BIT1|BIT2|BIT3);
            break;
        case 1:
            P3OUT = seg[adc_value[1]]+BIT5; // 두 번째로 낮은 자리의 데이터
            P4OUT &= ~BIT1;
            P4OUT |= (BIT0|BIT2|BIT3);
            break;
        }
    }
    else if(state == 2){
        cnt++;
        if(cnt > 1)
            cnt = 0;
        switch (cnt)
        {
        case 0:
            P3OUT = seg[adc_value[0]]; // 가장 낮은 자리의 데이터
            P4OUT &= ~BIT0;
            P4OUT |= (BIT1|BIT2|BIT3);
            break;
        case 1:
            P3OUT = seg[adc_value[1]]+BIT5; // 두 번째로 낮은 자리의 데이터
            P4OUT &= ~BIT1;
            P4OUT |= (BIT0|BIT2|BIT3);
            break;
        }
    }
    else if(state == 3)
        cnt = 4;
    else if(state == 4){
        cnt++;
        if(cnt > 3)
            cnt = 0;
        switch (cnt)
        {
        case 0:
            P3OUT = seg[adc_value[0]]; // 가장 낮은 자리의 데이터
            P4OUT &= ~BIT0;
            P4OUT |= (BIT1|BIT2|BIT3);
            break;
        case 1:
            P3OUT = seg[adc_value[1]]; // 두 번째로 낮은 자리의 데이터
            P4OUT &= ~BIT1;
            P4OUT |= (BIT0|BIT2|BIT3);
            break;
        case 2:
            P3OUT = seg[adc_value[2]]; // 세 번째로 낮은 자리의 데이터
            P4OUT &= ~BIT2;
            P4OUT |= (BIT0|BIT1|BIT3);
            break;
        case 3:
            P3OUT = seg[adc_value[3]]; // 가장 높은 자리의 데이터
            P4OUT &= ~BIT3;
            P4OUT |= (BIT0|BIT1|BIT2);
            break;
        }
    }
    else if(state == 5)
    {
        a++;
        if(a >= 100) //0.1초의 시간 간격으로
        {
            if(encoder_count < 100) //엔코더 카운트가 100 이하라면 모터 정지
            {
                TA2CCR1 = 0;
                TA2CCR2 = 0;
                motor_for = 0;
                motor_rev = 0;
            }
            encoder_count = 0;
            a = 0;
        }
    }


    if(state == 2)
    {
        led_cnt++;
        if (led_cnt >= led_cnt_req)
        {
            P1OUT ^= BIT0;    // P1.0 토글
            P4OUT ^= BIT7;    // P4.7 토글
            led_cnt = 0;      // led_cnt 초기화
        }
    }
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void)
{

    if (state == 4)
    {
        cho_timer++; // 초음파 센서 타이머 값 증가
        if(cho_stop == 1 && cho_timer > 1000) // 50ms 이상이면
        {
            cho_stop = 0; // cho_stop를 0으로 내리고 trig on 준비
        }
    }
}

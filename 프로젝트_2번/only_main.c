////////////////////////////////////

// ### Original Functions Prototypes
unsigned int scale_transform(int input);

// ### Main ###
void main(void) {
    stop_watchdog_timer();

    init_left_switch();
    init_7_segment();
    init_ADC_single_mode();

    enable_interrupt_vector();
    while (1)
    {
        adc_data = scale_transform(adc_data)
    }
}

// ### Original Function declare
unsigned int scale_transform(int input) {
    if (input < ADC_MIN || input > ADC_MAX) {
        return 1111; // out of range Error
    }

    int quotient = 0;
    int dividend = 10 * (input - ADC_MIN);

    while ((dividend - ADC_DELTA_TEN_TIME) >= 0) {
        dividend -= ADC_DELTA_TEN_TIME;
        quotient++;
    }
    return quotient;
}
// ### override functions
void left_switch_interrupt_handler(void){
    
    ADC_single_read(&adc_data);
}

void right_switch_interrupt_handler(void){
    // write your code here
}

///////////////////////////////////////
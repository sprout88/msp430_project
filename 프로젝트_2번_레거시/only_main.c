////////////////////////////////////

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

// ### override functions
void left_switch_interrupt_handler(void){
    
    ADC_single_read(&adc_data);
}

void right_switch_interrupt_handler(void){
    // write your code here
}

///////////////////////////////////////
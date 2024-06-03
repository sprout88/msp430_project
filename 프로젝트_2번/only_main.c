////////////////////////////////////

// ### Original Functions Prototypes
unsigned int scaled_linear_transform(int value, int scale, int in_min, int in_max, int out_min, int out_max);

// ### Main ###
void main(void) {
    stop_watchdog_timer();

    init_left_switch();
    init_7_segment();
    init_ADC_single_mode();

    enable_interrupt_vector();
    while (1)
    {
        adc_data = scaled_linear_transform(adc_data, 10, ADC_MIN, ADC_MAX, LINEAR_MAX, LINEAR_MIN);
    }
}

// ### Original Function declare
unsigned int scaled_linear_transform(int value, int scale, int in_min, int in_max, int out_min, int out_max) {
    if (value < in_min || value > in_max) {
        return 1111; // out of range Error
    }
    int linear_transformed_value = (value - in_min) * (out_max - out_min) * scale / (in_max - in_min) + (out_min * scale);
    return linear_transformed_value;
}

// ### override functions
void left_switch_interrupt_handler(void){
    
    ADC_single_read(&adc_data);
}

void right_switch_interrupt_handler(void){
    // write your code here
}

///////////////////////////////////////
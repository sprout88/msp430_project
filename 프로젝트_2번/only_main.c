// ### Main ###
void main(void) {
    stop_watchdog_timer();

    init_left_switch();
    init_7_segment();

    enable_interrupt_vector();
    while (1)
    {
        show_screen(adc_data);
    }
}

void left_switch_interrupt_handler(void){
    ADC_single_read(&adc_data);
}
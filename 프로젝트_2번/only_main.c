// ### Main ###
void main(void) {
    stop_watchdog_timer();

    init_left_switch();
    init_7_segment();

    enable_interrupt_vector();
    while (1)
    {
        
    }
}
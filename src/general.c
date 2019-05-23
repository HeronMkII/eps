#include "general.h"


// Initializes everything in EPS
void init_eps(void) {
    // UART
    init_uart();

    // SPI
    init_spi();

    // ADC
    init_adc(&adc);

    // PEX
    init_pex(&pex);

    // DAC
    init_dac(&dac);

    // Shunts
    init_shunts();

    // IMU
    init_imu();

    // Queues
    init_queue(&can_rx_msg_queue);
    init_queue(&can_tx_msg_queue);

    // CAN and MOBs
    init_can();
    init_rx_mob(&cmd_rx_mob);
    init_tx_mob(&data_tx_mob);
}


// If there is an RX messsage in the queue, handle it
void process_next_rx_msg(void) {
    if (!queue_empty(&can_rx_msg_queue)) {
        handle_rx_msg();
    }
}

/*
If there is a TX message in the queue, send it

When resume_mob(mob name) is called, it:
1) resumes the MOB
2) triggers an interrupt (callback function) to get the data to transmit
3) sends the data
4) pauses the mob
*/
// Checks the TX message queue and sends the first message (if it exists)
void send_next_tx_msg(void) {
    if (!queue_empty(&can_tx_msg_queue)) {
        resume_mob(&data_tx_mob);
    }
}

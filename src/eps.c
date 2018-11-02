#include "eps.h"

pin_info_t adc_cs = {
    .port = &ADC_CS_PORT_EPS,
    .ddr = &ADC_CS_DDR_EPS,
    .pin = ADC_CS_PIN_EPS
};

adc_t adc = {
    .channels = 0x0fff, // poll channels 0-11 in auto-1 mode
    .cs = &adc_cs
};


pin_info_t pex_cs = {
    .port = &PEX_CS_PORT_EPS,
    .ddr = &PEX_CS_DDR_EPS,
    .pin = PEX_CS_PIN_EPS
};

pin_info_t pex_rst = {
    .port = &PEX_RST_PORT_EPS,
    .ddr = &PEX_RST_DDR_EPS,
    .pin = PEX_RST_PIN_EPS
};

pex_t pex = {
    .addr = PEX_ADDR_EPS,
    .cs = &pex_cs,
    .rst = &pex_rst
};


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

    // Shunts
    init_shunts();

    // Queues
    init_queue(&can_rx_msgs);
    init_queue(&can_tx_msgs);

    // CAN and MOBs
    init_can();
    init_rx_mob(&cmd_rx_mob);
    init_tx_mob(&data_tx_mob);
}


// If there is an RX messsage in the queue, handle it
void process_next_rx_msg(void) {
    if (!queue_empty(&can_rx_msgs)) {
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
    if (!queue_empty(&can_tx_msgs)) {
        resume_mob(&data_tx_mob);
    }
}

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
    init_tx_mob(&cmd_tx_mob);

    init_uptime();
    init_com_timeout();
}

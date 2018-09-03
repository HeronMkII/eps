// Standard libraries
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// AVR libraries
#define F_CPU 8000000L
#include <util/delay.h>

// lib-common libraries
#include <queue/queue.h>
#include <spi/spi.h>
#include <uart/uart.h>
#include <adc/adc.h>
#include <pex/pex.h>

#include "shunts.h"
#include "can_eps.h"


// Initializes everything in EPS
void init_eps(void) {
    // UART
    init_uart();

    // SPI
    init_spi();

    // // ADC
    // adc_init(ADC_EPS);
    //
    // // PEX
    // pex_init(PEX_EPS);
    //
    // // Shunts
    // init_shunts();

    // Queues
    init_queue(&rx_messages);
    init_queue(&tx_messages);

    // CAN and MOBs
    init_can();
    init_rx_mob(&cmd_rx_mob);
    init_tx_mob(&data_tx_mob);
}


int main(void) {
    init_eps();
    print("\n\nEPS Initialized\n\n");

    // Main loop (infinite)
    while (1) {
        // TODO - control shunts

        // Process an RX CAN message
        process_rx();

        // Process a TX CAN message
        process_tx();
    }
}

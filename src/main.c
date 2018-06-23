#include "main.h"


void handle_rx(void) {
    if (is_empty(&rx_message_queue)) {
        print("RX queue empty\n");
        return;
    }

    // Received message
    uint8_t rx_data[8];
    dequeue(&rx_message_queue, rx_data);
    print("Dequeued RX\n");
    print_hex_bytes(rx_data, 8);

    // Check message type
    if (rx_data[1] == CAN_EPS_HK) {
        // TODO
        // uint16_t raw_data = adc_read_channel_raw_data(rx_data[2]);
        uint16_t raw_data = rand() % 32767;

        // Message to transmit
        uint8_t tx_data[8] = { 0 };

        // Send back the message type and field number
        tx_data[0] = 0; // TODO
        tx_data[1] = rx_data[1];
        tx_data[2] = rx_data[2];
        tx_data[3] = 0x00;
        tx_data[4] = (raw_data >> 8) & 0xFF;
        tx_data[5] = raw_data & 0xFF;

        // Enqueue TX data to transmit
        enqueue(&tx_message_queue, tx_data);
        print("Enqueued TX\n");
        print_hex_bytes(tx_data, 8);
    }

    else {
        print("Unknown message type\n");
    }
}


// Initializes everything in EPS
void init_eps(void) {
    // UART
    init_uart();
    print("\n\nInitializing EPS\n");
    print("UART\n");

    // SPI
    init_spi();
    print("SPI\n");

    // ADC
    adc_init(ADC_EPS);
    print("ADC\n");

    // PEX
    pex_init(PEX_EPS);
    print("PEX\n");

    // Shunts
    init_shunts();
    print("Shunts\n");

    // Queues
    init_queue(&rx_message_queue);
    init_queue(&tx_message_queue);
    print("Queues Initialized\n");

    // CAN and MOBs
    init_can();
    init_rx_mob(&cmd_rx_mob);
    init_tx_mob(&data_tx_mob);
    print("CAN\n");
}




int main(void) {
    init_eps();
    print("----\n");
    print("EPS Initialized\n\n");

    // Main loop
    print("Starting main loop\n");

    while (1) {
        // control_shunts();

        // If there is an RX messsage in the queue, handle it
        if (!is_empty(&rx_message_queue)) {
            handle_rx();
        }

        /*
        If there is a TX message in the queue, send it

        When resume_mob(mob name) is called, it:
        1) resumes the MOB
        2) triggers an interrupt (callback function) to get the data to transmit
        3) sends the data
        4) pauses the mob
        */
        else if (!is_empty(&tx_message_queue)) {
            resume_mob(&data_tx_mob);
        }
    }
}

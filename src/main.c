#include "main.h"


void handle_rx(void) {
    if (is_empty(&rx_message_queue)) {
        print("No data in RX message queue\n");
        return;
    }

    // Received message
    uint8_t rx_data[8];
    dequeue(&rx_message_queue, rx_data);
    print("Dequeued RX Message\n");
    print_hex_bytes(rx_data, 8);

    // Message to transmit
    uint8_t tx_data[8];

    // Send back the message type and field number
    tx_data[0] = rx_data[0];
    tx_data[1] = rx_data[1];
    tx_data[2] = rx_data[2];

    // Fill the rest with zeros just in case
    for (uint8_t i = 3; i < 8; i++) {
        tx_data[i] = 0;
    }

    // // Check message type
    // switch (rx_data[1]) {
    //     uint16_t raw_data;
    //
    //     case CAN_EPS_HK:
    //         raw_data = adc_read_channel_raw_data(rx_data[2]);
    //         tx_data[3] = 0x00;
    //         tx_data[4] = (raw_data >> 8) & 0xFF;
    //         tx_data[5] = raw_data & 0xFF;
    //         break;
    //
    //     default:
    //         print("Unknown message type\n");
    //         break;
    // }

    // Enqueue TX data to transmit
    enqueue(&tx_message_queue, tx_data);
    print("Enqueued TX Message\n");
    print_hex_bytes(tx_data, 8);
}


// Initializes everything in EPS
void init_eps(void) {
    // UART
    init_uart();
    print("\n\nUART Initialized\n");

    // SPI
    init_spi();
    print("SPI Initialized\n");

    // ADC
    adc_init_constants(ADC_EPS); //initialize adc
    adc_init();
    print("ADC Initialized\n");

    // PEX
    pex_init_constants(PEX_EPS);
    pex_init();
    print("PEX Initialized\n");

    // Shunts
    init_shunts();
    print("Shunts Initialized\n");

    // CAN and MOBs
    init_can();
    init_rx_mob(&cmd_rx_mob);
    init_tx_mob(&data_tx_mob);
    print("CAN Initialized\n");

    // Queues
    init_queue(&rx_message_queue);
    init_queue(&tx_message_queue);
    print("Queues Initialized\n");
}




int main(void) {
    init_eps();
    print("---------------\n");
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

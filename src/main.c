
// TODO - consider implementing function error checking (e.g. return 1 for success, 0 for failure)

#include "main.h"

// CAN messages received but not processed yet
queue_t rx_message_queue;
// CAN messages to transmit
queue_t tx_message_queue;




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

    // Check message type
    switch (rx_data[1]) {
        uint16_t raw_data;

        case CAN_EPS_HK:
            raw_data = adc_read_channel_raw_data(rx_data[2]);
            tx_data[3] = 0x00;
            tx_data[4] = (raw_data >> 8) & 0xFF;
            tx_data[5] = raw_data & 0xFF;
            break;

        default:
            print("Unknown message type\n");
            break;
    }

    // TODO - should it not transmit if the received message is not recognized?

    // Enqueue TX data to transmit
    enqueue(&tx_message_queue, tx_data);
    print("Enqueued TX Message\n");
    print_hex_bytes(tx_data, 8);
}




/* CAN Interrupts */


// MOB 0
// For heartbeat
void status_rx_callback(const uint8_t* data, uint8_t len) {
    print("MOB 0: Status RX Callback\n");
    print("Received Message:\n");
    print_hex_bytes((uint8_t *) data, len);
}


// MOB 1
// For heartbeat
void status_tx_callback(uint8_t* data, uint8_t* len) {
    print("MOB 1: Status TX Callback\n");
}


// MOB 2
// For later science requests?
void cmd_tx_callback(uint8_t* data, uint8_t* len) {
    print("MOB 2: CMD TX Callback\n");
}


// MOB 4
// CAN RX interrupt for received commands
void cmd_rx_callback(const uint8_t* data, uint8_t len) {
    print("\n\n\n\nMOB 4: CMD RX Callback\n");
    print("Received Message:\n");
    print_hex_bytes((uint8_t *) data, len);

    // TODO - would this ever happen?
    if (len == 0) {
        print("Received empty message\n");
    }

    // If the RX message exists, add it to the queue of received messages to process
    else {
        enqueue(&rx_message_queue, (uint8_t *) data);
        print("Enqueued RX message");
    }
}


// MOB 5
// CAN TX interrupt for sending data
void data_tx_callback(uint8_t* data, uint8_t* len) {
    print("\nData TX Callback\n");

    // TODO - would this ever happen?
    if (is_empty(&tx_message_queue)) {
        *len = 0;

        print("No message to transmit\n");
    }

    // If there is a message in the TX queue, transmit it
    else {
        dequeue(&tx_message_queue, data);
        *len = 8;

        print("Dequeued TX Message\n");
        print("Transmitting Message:\n");
        print_hex_bytes(data, *len);
    }
}




// Initializes everything in PAY
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
    init_rx_mob(&status_rx_mob);
    init_tx_mob(&status_tx_mob);
    init_tx_mob(&cmd_tx_mob);
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
        control_shunts();

        // If there is an RX messsage in the queue, handle it
        if (!is_empty(&rx_message_queue)) {
            handle_rx();
        }

        /*
        If there is a TX message in the queue, send it
        TODO - should new data already be available to send instead of waiting for it?

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

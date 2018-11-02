#include "commands.h"

// Checks the RX message queue and processes the first message (if it exists)
void handle_rx_msg(void) {
    // If there are no RX messages in the queue, exit the function
    if (queue_empty(&can_rx_msgs)) {
        return;
    }

    // Received message
    uint8_t rx_data[8];
    dequeue(&can_rx_msgs, rx_data);
    print("Dequeued RX\n");
    print_bytes(rx_data, 8);

    // Check message type
    if (rx_data[1] == CAN_EPS_HK) {
        uint8_t channel = rx_data[2];
        fetch_channel(&adc, channel);
        uint16_t raw_data = read_channel(&adc, channel);

        // Message to transmit
        // Send back the message type and field number
        uint8_t tx_data[8] = { 0 };
        tx_data[0] = 0; // TODO
        tx_data[1] = rx_data[1];
        tx_data[2] = rx_data[2];
        tx_data[3] = 0x00;
        tx_data[4] = (raw_data >> 8) & 0xFF;
        tx_data[5] = raw_data & 0xFF;

        // Enqueue TX data to transmit
        enqueue(&can_tx_msgs, tx_data);
        print("Enqueued TX\n");
        print_bytes(tx_data, 8);
    }

    else {
        print("Unknown message type\n");
    }
}

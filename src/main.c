#include "main.h"


// CAN messages received but not processed yet
queue_t rx_message_queue;
// CAN messages to transmit
queue_t tx_message_queue;


// MOB 4
// CMD RX - received commands
void cmd_rx_callback(const uint8_t* data, uint8_t len) {
    print("\n\nMOB 4: CMD RX Callback\n");
    print_hex_bytes((uint8_t *) data, len);

    if (len == 0) {
        print("Received empty message\n");
    }

    // If the RX message exists, add it to the queue of received messages to process
    else {
		if (data[1] == CAN_EPS_HK) {
	        enqueue(&rx_message_queue, (uint8_t *) data);
	        print("Enqueued RX\n");
		}
    }
}


// MOB 5
// DATA TX - transmitting data
void data_tx_callback(uint8_t* data, uint8_t* len) {
    print("\n\nMOB 5: Data TX Callback\n");

    if (is_empty(&tx_message_queue)) {
        *len = 0;
        print("No message to transmit\n");
    }

    // If there is a message in the TX queue, transmit it
    else {
        dequeue(&tx_message_queue, data);
        *len = 8;

        print("Dequeued TX\n");
        print("Transmitting Message:\n");
        print_hex_bytes(data, *len);
    }
}


mob_t cmd_rx_mob = {
	.mob_num = 4,
	.mob_type = RX_MOB,
    .dlc = 8,
    .id_tag = EPS_CMD_RX_MOB_ID,
	.id_mask = { 0x0000 },
	// .id_mask = CAN_RX_MASK_ID,
    .ctrl = default_rx_ctrl,

    .rx_cb = cmd_rx_callback
};

mob_t data_tx_mob = {
    .mob_num = 5,
	.mob_type = TX_MOB,
    .id_tag = EPS_DATA_TX_MOB_ID,
    .ctrl = default_tx_ctrl,

    .tx_data_cb = data_tx_callback
};


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

    // // ADC
    // adc_init(ADC_EPS);
    // print("ADC\n");
    //
    // // PEX
    // pex_init(PEX_EPS);
    // print("PEX\n");
    //
    // // Shunts
    // init_shunts();
    // print("Shunts\n");

    // Queues
    init_queue(&rx_message_queue);
    init_queue(&tx_message_queue);
    print("Queues\n");

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

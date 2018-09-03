#include "can_eps.h"


// CAN messages received but not processed yet
queue_t rx_messages;
// CAN messages that need to be transmitted (when possible)
queue_t tx_messages;


// Checks the RX message queue and processes the first message (if it exists)
void process_rx(void) {
    // If there are no RX messages in the queue, exit the function
    if (queue_empty(&rx_messages)) {
        return;
    }

    // Received message
    uint8_t rx_data[8];
    dequeue(&rx_messages, rx_data);
    print("Dequeued RX\n");
    print_bytes(rx_data, 8);

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
        enqueue(&tx_messages, tx_data);
        print("Enqueued TX\n");
        print_bytes(tx_data, 8);
    }

    else {
        print("Unknown message type\n");
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
void process_tx(void) {
    if (!queue_empty(&tx_messages)) {
        resume_mob(&data_tx_mob);
    }
}








// CAN MOBs

// mob_t status_rx_mob = {
// 	.mob_num = 0,
// 	.mob_type = RX_MOB,
//     .dlc = 8,
//     .id_tag = PAY_STATUS_RX_MOB_ID,
// 	.id_mask = CAN_RX_MASK_ID,
//     .ctrl = default_rx_ctrl,
//
//     .rx_cb = status_rx_callback
// };
//
// mob_t status_tx_mob = {
//     .mob_num = 1,
// 	.mob_type = TX_MOB,
//     .id_tag = PAY_STATUS_TX_MOB_ID,
//     .ctrl = default_tx_ctrl,
//
//     .tx_data_cb = status_tx_callback
// };
//
// mob_t cmd_tx_mob = {
// 	.mob_num = 2,
// 	.mob_type = TX_MOB,
// 	.id_tag = PAY_CMD_TX_MOB_ID,
// 	.ctrl = default_tx_ctrl,
//
// 	.tx_data_cb = cmd_tx_callback
// };


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






/* CAN Interrupts */

// // MOB 0
// void status_rx_callback(const uint8_t* data, uint8_t len) {
//     print("MOB 0: Status RX Callback\n");
//     print("Received Message:\n");
//     print_bytes((uint8_t *) data, len);
// }
//
// // MOB 1
// void status_tx_callback(uint8_t* data, uint8_t* len) {
//     print("MOB 1: Status TX Callback\n");
// }
//
// // MOB 2
// void cmd_tx_callback(uint8_t* data, uint8_t* len) {
//     print("MOB 2: CMD TX Callback\n");
// }

// MOB 4
// CMD RX - received commands
void cmd_rx_callback(const uint8_t* data, uint8_t len) {
    print("\n\nMOB 4: CMD RX Callback\n");
    print_bytes((uint8_t *) data, len);

    if (len == 0) {
        print("Received empty message\n");
    }

    // If the RX message exists, add it to the queue of received messages to process
    else {
		if (data[1] == CAN_EPS_HK) {
	        enqueue(&rx_messages, (uint8_t *) data);
	        print("Enqueued RX\n");
		}
    }
}

// MOB 5
// DATA TX - transmitting data
void data_tx_callback(uint8_t* data, uint8_t* len) {
    print("\n\nMOB 5: Data TX Callback\n");

    if (queue_empty(&tx_messages)) {
        *len = 0;
        print("No message to transmit\n");
    }

    // If there is a message in the TX queue, transmit it
    else {
        dequeue(&tx_messages, data);
        *len = 8;

        print("Dequeued TX\n");
        print("Transmitting Message:\n");
        print_bytes(data, *len);
    }
}

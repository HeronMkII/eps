#include "can_interface.h"


/* CAN Interrupts */

// MOB 4
// CMD RX - received commands
void cmd_rx_callback(const uint8_t* data, uint8_t len) {
    if (len == 0) {
        return;
    }

    // If the RX message exists, add it to the queue of received messages to process
    enqueue(&can_rx_msg_queue, (uint8_t *) data);
}

// MOB 5
// DATA TX - transmitting data
void data_tx_callback(uint8_t* data, uint8_t* len) {
    if (queue_empty(&can_tx_msg_queue)) {
        *len = 0;
        return;
    }

    // If there is a message in the TX queue, transmit it
    dequeue(&can_tx_msg_queue, data);
    *len = 8;
}




// CAN MOBs

mob_t cmd_rx_mob = {
	.mob_num = EPS_CMD_MOB_NUM,
	.mob_type = RX_MOB,
    .dlc = 8,
    .id_tag = { EPS_EPS_CMD_MOB_ID },
	.id_mask = { CAN_RX_MASK_ID },
    .ctrl = default_rx_ctrl,

    .rx_cb = cmd_rx_callback
};

mob_t cmd_tx_mob = {
    .mob_num = OBC_CMD_MOB_NUM,
	.mob_type = TX_MOB,
    .id_tag = { EPS_OBC_CMD_MOB_ID },
    .ctrl = default_tx_ctrl,

    .tx_data_cb = data_tx_callback
};

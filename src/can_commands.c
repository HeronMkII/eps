/*
Defines the logical interface that EPS uses for CAN communication. This is for
handling received CAN messages, performing actions, and responding.

Authors: Bruno Almeida
*/

#include "can_commands.h"


// CAN messages received but not processed yet
queue_t can_rx_msg_queue;
// CAN messages that need to be transmitted (when possible)
queue_t can_tx_msg_queue;

// Set this to true to simulate performing all local actions (e.g. fetching
// data, actuating motors) - this allows testing just the PAY command handling
// system on any PCB without any peripherals
bool sim_local_actions = false;

void handle_rx_hk(uint8_t* rx_msg);
void handle_rx_ctrl(uint8_t* rx_msg);


// Checks the RX message queue and processes the first message (if it exists)
void handle_rx_msg(void) {
    // Received message
    uint8_t rx_msg[8] = {0x00};
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        // If there are no RX messages in the queue, exit the function
        if (queue_empty(&can_rx_msg_queue)) {
            return;
        }
        dequeue(&can_rx_msg_queue, rx_msg);
    }

    switch (rx_msg[1]) {
        case CAN_EPS_HK:
            handle_rx_hk(rx_msg);
            break;
        case CAN_EPS_CTRL:
            handle_rx_ctrl(rx_msg);
            break;
        default:
            break;
    }
}

void handle_rx_hk(uint8_t* rx_msg) {
    uint8_t field_num = rx_msg[2];
    uint32_t data = 0;

    // Check field number
    if ((CAN_EPS_HK_BB_VOL <= field_num) &&
            (field_num <= CAN_EPS_HK_BT_VOL)) {
        if (sim_local_actions) {
            // use 11 bits for ADC data
            data = random() & 0x7FF;
        } else {
            uint8_t channel = field_num - CAN_EPS_HK_BB_VOL;
            fetch_adc_channel(&adc, channel);
            data = read_adc_channel(&adc, channel);
        }
    }

    else if (field_num == CAN_EPS_HK_HEAT_SP1) {
        if (sim_local_actions) {
            data = random() & 0x7FF;
        } else {
            data = dac.raw_voltage_a;
        }
    }

    else if (field_num == CAN_EPS_HK_HEAT_SP2) {
        if (sim_local_actions) {
            data = random() & 0x7FF;
        } else {
            data = dac.raw_voltage_b;
        }
    }

    else if ((CAN_EPS_HK_IMU_ACC_X <= field_num) &&
            (field_num <= CAN_EPS_HK_IMU_MAG_Z)) {
        // TODO - get IMU data
        data = random() & 0x7FFF;
    }

    // If the message type is not recognized, return before enqueueing
    else {
        return;
    }

    // Message to transmit
    // Send back the message type and field number
    uint8_t tx_msg[8] = {0x00};
    tx_msg[0] = 0; // TODO
    tx_msg[1] = rx_msg[1];
    tx_msg[2] = rx_msg[2];
    tx_msg[3] = (data >> 16) & 0xFF;
    tx_msg[4] = (data >> 8) & 0xFF;
    tx_msg[5] = data & 0xFF;

    // Enqueue TX data to transmit
    enqueue(&can_tx_msg_queue, tx_msg);
    // print("Enqueued TX\n");
    // print_bytes(tx_msg, 8);
}


void handle_rx_ctrl(uint8_t* rx_msg) {
    uint8_t field_num = rx_msg[2];
    uint32_t data =
        (((uint32_t) rx_msg[3]) << 16) |
        (((uint32_t) rx_msg[4]) << 8) |
        ((uint32_t) rx_msg[5]);

    // Check field number
    if (field_num == CAN_EPS_CTRL_HEAT_SP1) {
        set_dac_raw_voltage(&dac, DAC_A, (uint16_t) data);
    }

    else if (field_num == CAN_EPS_CTRL_HEAT_SP2) {
        set_dac_raw_voltage(&dac, DAC_B, (uint16_t) data);
    }

    // If the field number is not recognized, return before enqueueing
    else {
        return;
    }

    // Message to transmit
    // Send back the message type and field number
    uint8_t tx_msg[8] = {0x00};
    tx_msg[0] = 0; // TODO
    tx_msg[1] = rx_msg[1];
    tx_msg[2] = rx_msg[2];

    // Enqueue TX data to transmit
    enqueue(&can_tx_msg_queue, tx_msg);
    // print("Enqueued TX\n");
    // print_bytes(tx_msg, 8);
}

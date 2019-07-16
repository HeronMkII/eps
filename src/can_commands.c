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

void handle_rx_hk(uint8_t field_num);
void handle_rx_ctrl(uint8_t field_num, uint32_t rx_data);


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

    uint8_t msg_type = rx_msg[2];
    uint8_t field_num = rx_msg[3];
    uint32_t rx_data =
        ((uint32_t) rx_msg[4] << 24) |
        ((uint32_t) rx_msg[5] << 16) |
        ((uint32_t) rx_msg[6] << 8) |
        ((uint32_t) rx_msg[7]);

    switch (msg_type) {
        case CAN_EPS_HK:
            handle_rx_hk(field_num);
            break;
        case CAN_EPS_CTRL:
            handle_rx_ctrl(field_num, rx_data);
            break;
        default:
            break;
    }
}

void handle_rx_hk(uint8_t field_num) {
    uint32_t tx_data = 0;

    // Check field number
    if ((CAN_EPS_HK_BB_VOL <= field_num) &&
            (field_num <= CAN_EPS_HK_BT_VOL)) {
        if (sim_local_actions) {
            // use 11 bits for ADC data
            tx_data = random() & 0x7FF;
        } else {
            uint8_t channel = field_num - CAN_EPS_HK_BB_VOL;
            fetch_adc_channel(&adc, channel);
            tx_data = read_adc_channel(&adc, channel);
        }
    }

    else if (field_num == CAN_EPS_HK_HEAT_SP1) {
        if (sim_local_actions) {
            tx_data = random() & 0x7FF;
        } else {
            tx_data = dac.raw_voltage_a;
        }
    }

    else if (field_num == CAN_EPS_HK_HEAT_SP2) {
        if (sim_local_actions) {
            tx_data = random() & 0x7FF;
        } else {
            tx_data = dac.raw_voltage_b;
        }
    }

    else if ((CAN_EPS_HK_GYR_UNCAL_X <= field_num) &&
            (field_num <= CAN_EPS_HK_GYR_UNCAL_Z)) {
        if (sim_local_actions) {
            tx_data = random() & 0x7FFF;
        } else {
            uint16_t uncal_x = 0, uncal_y = 0, uncal_z = 0;
            get_imu_uncal_gyro(&uncal_x, &uncal_y, &uncal_z, NULL, NULL, NULL);

            switch (field_num) {
                case CAN_EPS_HK_GYR_UNCAL_X:
                    tx_data = (uint32_t) uncal_x;
                    break;
                case CAN_EPS_HK_GYR_UNCAL_Y:
                    tx_data = (uint32_t) uncal_y;
                    break;
                case CAN_EPS_HK_GYR_UNCAL_Z:
                    tx_data = (uint32_t) uncal_z;
                    break;
                default:
                    break;
            }
        }
    }

    else if ((CAN_EPS_HK_GYR_CAL_X <= field_num) &&
            (field_num <= CAN_EPS_HK_GYR_CAL_Z)) {
        if (sim_local_actions) {
            tx_data = random() & 0x7FFF;
        } else {
            uint16_t cal_x = 0, cal_y = 0, cal_z = 0;
            get_imu_cal_gyro(&cal_x, &cal_y, &cal_z);

            switch (field_num) {
                case CAN_EPS_HK_GYR_CAL_X:
                    tx_data = (uint32_t) cal_x;
                    break;
                case CAN_EPS_HK_GYR_CAL_Y:
                    tx_data = (uint32_t) cal_y;
                    break;
                case CAN_EPS_HK_GYR_CAL_Z:
                    tx_data = (uint32_t) cal_z;
                    break;
                default:
                    break;
            }
        }
    }

    else if (field_num == CAN_EPS_HK_HEAT_SHADOW_SP1) {
        if (sim_local_actions) {
            tx_data = random() & 0x7FF;
        } else {
            tx_data = heater_1_shadow_setpoint.raw;
        }
    }

    else if (field_num == CAN_EPS_HK_HEAT_SHADOW_SP2) {
        if (sim_local_actions) {
            tx_data = random() & 0x7FF;
        } else {
            tx_data = heater_2_shadow_setpoint.raw;
        }
    }

    else if (field_num == CAN_EPS_HK_HEAT_SUN_SP1) {
        if (sim_local_actions) {
            tx_data = random() & 0x7FF;
        } else {
            tx_data = heater_1_sun_setpoint.raw;
        }
    }

    else if (field_num == CAN_EPS_HK_HEAT_SUN_SP2) {
        if (sim_local_actions) {
            tx_data = random() & 0x7FF;
        } else {
            tx_data = heater_2_sun_setpoint.raw;
        }
    }

    // If the message type is not recognized, return before enqueueing
    else {
        return;
    }

    // Message to transmit
    // Send back the message type and field number
    uint8_t tx_msg[8] = {0x00};
    tx_msg[0] = 0x00;
    tx_msg[1] = 0x00;
    tx_msg[2] = CAN_EPS_HK;
    tx_msg[3] = field_num;
    tx_msg[4] = (tx_data >> 24) & 0xFF;
    tx_msg[5] = (tx_data >> 16) & 0xFF;
    tx_msg[6] = (tx_data >> 8) & 0xFF;
    tx_msg[7] = tx_data & 0xFF;

    // Enqueue TX data to transmit
    enqueue(&can_tx_msg_queue, tx_msg);
    // print("Enqueued TX\n");
    // print_bytes(tx_msg, 8);
}


void handle_rx_ctrl(uint8_t field_num, uint32_t rx_data) {
    uint32_t tx_data = 0;

    // Check field number

    if (field_num == CAN_EPS_CTRL_PING) {
        // Don't need to do anything
        // Just take care of the condition so we don't return early below
    }
    
    else if (field_num == CAN_EPS_CTRL_HEAT_SHADOW_SP1) {
        set_raw_heater_setpoint(&heater_1_shadow_setpoint, (uint16_t) rx_data);
    }
    else if (field_num == CAN_EPS_CTRL_HEAT_SHADOW_SP2) {
        set_raw_heater_setpoint(&heater_2_shadow_setpoint, (uint16_t) rx_data);
    }
    else if (field_num == CAN_EPS_CTRL_HEAT_SUN_SP1) {
        set_raw_heater_setpoint(&heater_1_sun_setpoint, (uint16_t) rx_data);
    }
    else if (field_num == CAN_EPS_CTRL_HEAT_SUN_SP2) {
        set_raw_heater_setpoint(&heater_2_sun_setpoint, (uint16_t) rx_data);
    }

    else if (field_num == CAN_EPS_CTRL_HEAT_CUR_THRESH_LOWER) {
        set_raw_heater_cur_thresh(&heater_sun_cur_thresh_lower, (uint16_t) rx_data);
    }
    else if (field_num == CAN_EPS_CTRL_HEAT_CUR_THRESH_UPPER) {
        set_raw_heater_cur_thresh(&heater_sun_cur_thresh_upper, (uint16_t) rx_data);
    }

    else if (field_num == CAN_EPS_CTRL_RESET) {
        // Program will stop here and restart from the beginning
        reset_self_mcu(UPTIME_RESTART_REASON_RESET_CMD);
    }

    else if (field_num == CAN_EPS_CTRL_READ_EEPROM) {
        // Received rx_data as a uint32_t but need to represent it as a uint32_t*, which the eeprom function requires
        // Note that sizeof(uint32_t) = 4, sizeof(uint32_t*) = 2
        // Need to case to uint16_T first or else we get
        // warning: cast to pointer from integer of different size [-Wint-to-pointer-cast]
        uint32_t* ptr = (uint32_t*) ((uint16_t) rx_data);
        tx_data = eeprom_read_dword(ptr);
    }

    else if (field_num == CAN_EPS_CTRL_RESTART_COUNT) {
        tx_data = restart_count;
    }
    else if (field_num == CAN_EPS_CTRL_RESTART_REASON) {
        tx_data = restart_reason;
    }
    else if (field_num == CAN_EPS_CTRL_UPTIME) {
        tx_data = uptime_s;
    }

    // If the field number is not recognized, return before enqueueing so we
    // don't send anything back
    else {
        return;
    }

    // Message to transmit
    // Send back the message type and field number
    uint8_t tx_msg[8] = {0x00};
    tx_msg[0] = 0x00;
    tx_msg[1] = 0x00;
    tx_msg[2] = CAN_EPS_HK;
    tx_msg[3] = field_num;
    tx_msg[4] = (tx_data >> 24) & 0xFF;
    tx_msg[5] = (tx_data >> 16) & 0xFF;
    tx_msg[6] = (tx_data >> 8) & 0xFF;
    tx_msg[7] = tx_data & 0xFF;

    // Enqueue TX data to transmit
    enqueue(&can_tx_msg_queue, tx_msg);
    // print("Enqueued TX\n");
    // print_bytes(tx_msg, 8);
}

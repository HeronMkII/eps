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

void handle_rx_hk(uint8_t field_num, uint8_t* tx_status, uint32_t* tx_data);
void handle_rx_ctrl(uint8_t field_num, uint32_t rx_data, uint8_t* tx_status,
        uint32_t* tx_data);


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

    uint8_t opcode = rx_msg[0];
    uint8_t field_num = rx_msg[1];
    uint32_t rx_data =
        ((uint32_t) rx_msg[4] << 24) |
        ((uint32_t) rx_msg[5] << 16) |
        ((uint32_t) rx_msg[6] << 8) |
        ((uint32_t) rx_msg[7]);
    
    uint8_t tx_status = CAN_STATUS_OK;
    uint32_t tx_data = 0;

    switch (opcode) {
        case CAN_EPS_HK:
            handle_rx_hk(field_num, &tx_status, &tx_data);
            break;
        case CAN_EPS_CTRL:
            handle_rx_ctrl(field_num, rx_data, &tx_status, &tx_data);
            break;
        default:
            tx_status = CAN_STATUS_INVALID_OPCODE;
            break;
    }

    // Message to transmit
    // Send back the message type and field number
    uint8_t tx_msg[8] = {0x00};
    tx_msg[0] = opcode;
    tx_msg[1] = field_num;
    tx_msg[2] = tx_status;
    tx_msg[3] = 0x00;
    tx_msg[4] = (tx_data >> 24) & 0xFF;
    tx_msg[5] = (tx_data >> 16) & 0xFF;
    tx_msg[6] = (tx_data >> 8) & 0xFF;
    tx_msg[7] = tx_data & 0xFF;
    // Enqueue TX data to transmit
    enqueue(&can_tx_msg_queue, tx_msg);

    restart_com_timeout();
}

void handle_rx_hk(uint8_t field_num, uint8_t* tx_status, uint32_t* tx_data) {
    // TODO - field implementations

    // Check field number
    if (field_num == CAN_EPS_HK_BAT_VOL) {
        *tx_data = fetch_and_read_adc_channel(&adc, ADC_VMON_PACK);
    }

    else if (field_num == CAN_EPS_HK_BAT_CUR) {
        *tx_data = fetch_and_read_adc_channel(&adc, ADC_IMON_PACK);
    }

    else if (field_num == CAN_EPS_HK_X_POS_CUR) {
        *tx_data = fetch_and_read_adc_channel(&adc, ADC_IMON_X_PLUS);
    }

    else if (field_num == CAN_EPS_HK_X_NEG_CUR) {
        *tx_data = fetch_and_read_adc_channel(&adc, ADC_IMON_X_MINUS);
    }

    else if (field_num == CAN_EPS_HK_Y_POS_CUR) {
        *tx_data = fetch_and_read_adc_channel(&adc, ADC_IMON_Y_PLUS);
    }

    else if (field_num == CAN_EPS_HK_Y_NEG_CUR) {
        *tx_data = fetch_and_read_adc_channel(&adc, ADC_IMON_Y_MINUS);
    }

    else if (field_num == CAN_EPS_HK_3V3_VOL) {
        *tx_data = fetch_and_read_adc_channel(&adc, ADC_VMON_3V3);
    }

    else if (field_num == CAN_EPS_HK_3V3_CUR) {
        *tx_data = fetch_and_read_adc_channel(&adc, ADC_IMON_3V3);
    }

    else if (field_num == CAN_EPS_HK_5V_VOL) {
        *tx_data = fetch_and_read_adc_channel(&adc, ADC_VMON_5V);
    }

    else if (field_num == CAN_EPS_HK_5V_CUR) {
        *tx_data = fetch_and_read_adc_channel(&adc, ADC_IMON_5V);
    }

    // TODO implement conversion
    else if (field_num == CAN_EPS_HK_PAY_CUR) {
        *tx_data = fetch_and_read_adc_channel(&adc, ADC_IMON_PAY_LIM);
    }

    else if (field_num == CAN_EPS_HK_BAT_TEMP1) {
        *tx_data = fetch_and_read_adc_channel(&adc, ADC_THM_BATT1);
    }

    else if (field_num == CAN_EPS_HK_BAT_TEMP2) {
        *tx_data = fetch_and_read_adc_channel(&adc, ADC_THM_BATT2);
    }

    else if (field_num == CAN_EPS_HK_3V3_TEMP) {
        *tx_data = fetch_and_read_adc_channel(&adc, ADC_THM_3V3_TOP);
    }

    else if (field_num == CAN_EPS_HK_5V_TEMP) {
        *tx_data = fetch_and_read_adc_channel(&adc, ADC_THM_5V_TOP);
    }

    else if (field_num == CAN_EPS_HK_PAY_CON_TEMP) {
        *tx_data = fetch_and_read_adc_channel(&adc, ADC_THM_PAY_CONN);
    }

    else if (field_num == CAN_EPS_HK_SHUNTS) {
    }

    else if (field_num == CAN_EPS_HK_HEAT1_SP) {
        *tx_data = dac.raw_voltage_a;
    }

    else if (field_num == CAN_EPS_HK_HEAT2_SP) {
        *tx_data = dac.raw_voltage_b;
    }    

    else if (field_num == CAN_EPS_HK_GYR_UNCAL_X) {
        uint16_t uncal_x = 0;
        get_imu_uncal_gyro(&uncal_x, NULL, NULL, NULL, NULL, NULL);
        *tx_data = (uint32_t) uncal_x;
    }

    else if (field_num == CAN_EPS_HK_GYR_UNCAL_Y) {
        uint16_t uncal_y = 0;
        get_imu_uncal_gyro(NULL, &uncal_y, NULL, NULL, NULL, NULL);
        *tx_data = (uint32_t) uncal_y;
    }

    else if (field_num == CAN_EPS_HK_GYR_UNCAL_Z) {
        uint16_t uncal_z = 0;
        get_imu_uncal_gyro(NULL, NULL, &uncal_z, NULL, NULL, NULL);
        *tx_data = (uint32_t) uncal_z;
    }

    else if (field_num == CAN_EPS_HK_GYR_CAL_X) {
        uint16_t cal_x = 0;
        get_imu_cal_gyro(&cal_x, NULL, NULL);
        *tx_data = (uint32_t) cal_x;
    }

    else if (field_num == CAN_EPS_HK_GYR_CAL_Y) {
        uint16_t cal_y = 0;
        get_imu_cal_gyro(NULL, &cal_y, NULL);
        *tx_data = (uint32_t) cal_y;
    }

    else if (field_num == CAN_EPS_HK_GYR_CAL_Z) {
        uint16_t cal_z = 0;
        get_imu_cal_gyro(NULL, NULL, &cal_z);
        *tx_data = (uint32_t) cal_z;
    }

    else if (field_num == CAN_EPS_HK_UPTIME) {
        *tx_data = uptime_s;
    }

    else if (field_num == CAN_EPS_HK_RESTART_COUNT) {
        *tx_data = restart_count;
    }

    else if (field_num == CAN_EPS_HK_RESTART_REASON) {
        *tx_data = restart_reason;
    }

    // If the message type is not recognized, return before enqueueing
    else {
        *tx_status = CAN_STATUS_INVALID_FIELD_NUM;
    }
}


void handle_rx_ctrl(uint8_t field_num, uint32_t rx_data, uint8_t* tx_status,
        uint32_t* tx_data) {
    // TODO - field implementations

    if (field_num == CAN_EPS_CTRL_PING) {
        // Don't need to do anything
        // Just take care of the condition
    }
    
    else if (field_num == CAN_EPS_CTRL_GET_HEAT1_SHAD_SP) {
        *tx_data = heater_1_shadow_setpoint.raw;
    }

    else if (field_num == CAN_EPS_CTRL_SET_HEAT1_SHAD_SP) {
        set_raw_heater_setpoint(&heater_1_shadow_setpoint, (uint16_t) rx_data);
    }

    else if (field_num == CAN_EPS_CTRL_GET_HEAT2_SHAD_SP) {
        *tx_data = heater_2_shadow_setpoint.raw;
    }

    else if (field_num == CAN_EPS_CTRL_SET_HEAT2_SHAD_SP) {
        set_raw_heater_setpoint(&heater_2_shadow_setpoint, (uint16_t) rx_data);
    }

    else if (field_num == CAN_EPS_CTRL_GET_HEAT1_SUN_SP) {
        *tx_data = heater_1_sun_setpoint.raw;
    }

    else if (field_num == CAN_EPS_CTRL_SET_HEAT1_SUN_SP) {
        set_raw_heater_setpoint(&heater_1_sun_setpoint, (uint16_t) rx_data);
    }

    else if (field_num == CAN_EPS_CTRL_GET_HEAT2_SUN_SP) {
        *tx_data = heater_2_sun_setpoint.raw;
    }

    else if (field_num == CAN_EPS_CTRL_SET_HEAT2_SUN_SP) {
        set_raw_heater_setpoint(&heater_2_sun_setpoint, (uint16_t) rx_data);
    }

    else if (field_num == CAN_EPS_CTRL_GET_HEAT_CUR_THRESH_LOWER) {
        *tx_data = heater_sun_cur_thresh_lower.raw;
    }

    else if (field_num == CAN_EPS_CTRL_SET_HEAT_CUR_THRESH_LOWER) {
        set_raw_heater_cur_thresh(&heater_sun_cur_thresh_lower, (uint16_t) rx_data);
    }

    else if (field_num == CAN_EPS_CTRL_GET_HEAT_CUR_THRESH_UPPER) {
        *tx_data = heater_sun_cur_thresh_upper.raw;
    }

    else if (field_num == CAN_EPS_CTRL_SET_HEAT_CUR_THRESH_UPPER) {
        set_raw_heater_cur_thresh(&heater_sun_cur_thresh_upper, (uint16_t) rx_data);
    }

    else if (field_num == CAN_EPS_CTRL_RESET) {
        // Program will stop here and restart from the beginning
        reset_self_mcu(UPTIME_RESTART_REASON_RESET_CMD);
    }

    else if (field_num == CAN_EPS_CTRL_READ_EEPROM) {
        *tx_data = read_eeprom((uint16_t) rx_data);
    }

    else if (field_num == CAN_EPS_CTRL_ERASE_EEPROM) {
        write_eeprom((uint16_t) rx_data, EEPROM_DEF_DWORD);
    }

    else if (field_num == CAN_EPS_CTRL_READ_RAM_BYTE) {
        // See lib-common/examples/read_registers for an MMIO example
        // https://arduino.stackexchange.com/questions/56304/how-do-i-directly-access-a-memory-mapped-register-of-avr-with-c
        // http://download.mikroe.com/documents/compilers/mikroc/avr/help/avr_memory_organization.htm

        // Need to represent address as volatile uint8_t* to read RAM
        // Must first cast to uint16_t or else we get warning: cast to pointer
        // from integer of different size -Wint-to-pointer-cast]
        volatile uint8_t* pointer = (volatile uint8_t*) ((uint16_t) rx_data);
        *tx_data = (uint32_t) (*pointer);
    }

    // If the field number is not recognized, return before enqueueing so we
    // don't send anything back
    else {
        *tx_status = CAN_STATUS_INVALID_FIELD_NUM;
    }
}

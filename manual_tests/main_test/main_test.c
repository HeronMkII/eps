/*
A test for all EPS functionality by simulating sending and receiving CAN messages.
RX and TX are defined from EPS's perspective.
*/

#include <can/can.h>
#include <can/data_protocol.h>
#include <can/ids.h>
#include <uart/uart.h>

#include "../../src/general.h"

// Set to true to simulate OBC's CAN messages
bool sim_obc = false;
bool disable_hb = false;
// Set to true to print EPS's TX and RX CAN messages
bool print_can_msgs = false;


// Callback function signature to run a command
typedef void(*uart_cmd_fn_t)(void);

// UART-activated command
typedef struct {
    char* description;
    uart_cmd_fn_t fn;
} uart_cmd_t;


void get_status_fn(void);
void req_eps_hk_fn(void);
void heater_1_low_fn(void);
void heater_1_high_fn(void);
void heater_2_low_fn(void);
void heater_2_high_fn(void);
void reset_fn(void);

// All possible commands
uart_cmd_t all_cmds[] = {
    {
        .description = "Get subsystem status",
        .fn = get_status_fn
    },
    {
        .description = "Request EPS HK data",
        .fn = req_eps_hk_fn
    },
    {
        .description = "Set heater 1 setpoint = 0C/-20C",
        .fn = heater_1_low_fn
    },
    {
        .description = "Set heater 1 setpoint = 100C/65C",
        .fn = heater_1_high_fn
    },
    {
        .description = "Set heater 2 setpoint = 0C/-20C",
        .fn = heater_2_low_fn
    },
    {
        .description = "Set heater 2 setpoint = 100C/65C",
        .fn = heater_2_high_fn
    },
    {
        .description = "Reset subsystem",
        .fn = reset_fn
    },
};
// Length of array
const uint8_t all_cmds_len = sizeof(all_cmds) / sizeof(all_cmds[0]);




// Enqueues a message for EPS to receive
void enqueue_rx_msg(uint8_t msg_type, uint8_t field_number, uint32_t raw_data) {
    uint8_t rx_msg[8] = { 0x00 };
    rx_msg[0] = 0x00;
    rx_msg[1] = 0x00;
    rx_msg[2] = msg_type;
    rx_msg[3] = field_number;
    rx_msg[4] = (raw_data >> 24) & 0xFF;
    rx_msg[5] = (raw_data >> 16) & 0xFF;
    rx_msg[6] = (raw_data >> 8) & 0xFF;
    rx_msg[7] = raw_data & 0xFF;
    enqueue(&can_rx_msg_queue, rx_msg);
}


void print_voltage(uint16_t raw_data) {
    print(" 0x%.3X = %.3f V\n", raw_data, adc_raw_data_to_eps_vol(raw_data));
}

void print_current(uint16_t raw_data) {
    print(" 0x%.3X = %.3f A\n", raw_data, adc_raw_data_to_eps_cur(raw_data));
}

void print_bat_current(uint16_t raw_data) {
    print(" 0x%.3X = %.3f A\n", raw_data, adc_raw_data_to_bat_cur(raw_data));
}

void print_therm_temp(uint16_t raw_data) {
    print(" 0x%.3X = %.2f C\n", raw_data, adc_raw_data_to_therm_temp(raw_data));
}

void print_imu_gyro(uint16_t raw_data) {
    print(" 0x%.4X = %.3f rad/s\n", raw_data, imu_raw_data_to_gyro(raw_data));
}

void process_eps_hk_tx_msg(uint8_t field_num, uint32_t tx_data) {
    switch (field_num) {
        case CAN_EPS_HK_3V3_VOL:
            print("BB Vol:");
            print_voltage(tx_data);
            break;
        case CAN_EPS_HK_3V3_CUR:
            print("BB Cur:");
            print_current(tx_data);
            break;
        case CAN_EPS_HK_Y_NEG_CUR:
            print("-Y Cur:");
            print_current(tx_data);
            break;
        case CAN_EPS_HK_X_POS_CUR:
            print("+X Cur:");
            print_current(tx_data);
            break;
        case CAN_EPS_HK_Y_POS_CUR:
            print("+Y Cur:");
            print_current(tx_data);
            break;
        case CAN_EPS_HK_X_NEG_CUR:
            print("-X Cur:");
            print_current(tx_data);
            break;
        case CAN_EPS_HK_BAT_TEMP1:
            print("Bat Temp 1:");
            print_therm_temp(tx_data);
            break;
        case CAN_EPS_HK_BAT_TEMP2:
            print("Bat Temp 2:");
            print_therm_temp(tx_data);
            break;
        case CAN_EPS_HK_BAT_VOL:
            print("Bat Vol:");
            print_voltage(tx_data);
            break;
        case CAN_EPS_HK_BAT_CUR:
            print("Bat Cur:");
            print_bat_current(tx_data);
            break;
        case CAN_EPS_HK_5V_CUR:
            print("BT Cur:");
            print_current(tx_data);
            break;
        case CAN_EPS_HK_5V_VOL:
            print("BT Vol:");
            print_voltage(tx_data);
            break;
        case CAN_EPS_HK_HEAT1_SP:
            print("Heater Setpoint 1:");
            print_therm_temp(tx_data);
            break;
        case CAN_EPS_HK_HEAT2_SP:
            print("Heater Setpoint 2:");
            print_therm_temp(tx_data);
            break;
        case CAN_EPS_HK_GYR_UNCAL_X:
            print("Gyro (Uncal) X:");
            print_imu_gyro(tx_data);
            break;
        case CAN_EPS_HK_GYR_UNCAL_Y:
            print("Gyro (Uncal) Y:");
            print_imu_gyro(tx_data);
            break;
        case CAN_EPS_HK_GYR_UNCAL_Z:
            print("Gyro (Uncal) Z:");
            print_imu_gyro(tx_data);
            break;
        case CAN_EPS_HK_GYR_CAL_X:
            print("Gyro (Cal) X:");
            print_imu_gyro(tx_data);
            break;
        case CAN_EPS_HK_GYR_CAL_Y:
            print("Gyro (Cal) Y:");
            print_imu_gyro(tx_data);
            break;
        case CAN_EPS_HK_GYR_CAL_Z:
            print("Gyro (Cal) Z:");
            print_imu_gyro(tx_data);
            break;
        default:
            return;
    }

    uint8_t next_field_num = field_num + 1;
    if (next_field_num < CAN_EPS_HK_FIELD_COUNT) {
        enqueue_rx_msg(CAN_EPS_HK, next_field_num, 0);
    }
}


void process_eps_ctrl_tx_msg(uint8_t field_num) {
    switch (field_num) {
        case CAN_EPS_CTRL_SET_HEAT1_SHAD_SP:
            print("Set heater - shadow setpoint 1\n");
            break;
        case CAN_EPS_CTRL_SET_HEAT2_SHAD_SP:
            print("Set heater - shadow setpoint 2\n");
            break;
        case CAN_EPS_CTRL_SET_HEAT1_SUN_SP:
            print("Set heater - sun setpoint 1\n");
            break;
        case CAN_EPS_CTRL_SET_HEAT2_SUN_SP:
            print("Set heater - sun setpoint 2\n");
            break;
        default:
            break;
    }
}


// Displays the response that EPS sends back
void sim_send_next_tx_msg(void) {
    uint8_t tx_msg[8] = { 0x00 };
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (queue_empty(&can_tx_msg_queue)) {
            return;
        }
        dequeue(&can_tx_msg_queue, tx_msg);
    }

    uint8_t msg_type = tx_msg[2];
    uint8_t field_num = tx_msg[3];
    uint32_t tx_data =
        ((uint32_t) tx_msg[4] << 24) |
        ((uint32_t) tx_msg[5] << 16) |
        ((uint32_t) tx_msg[6] << 8) |
        ((uint32_t) tx_msg[7]);

    switch (msg_type) {
        case CAN_EPS_HK:
            process_eps_hk_tx_msg(field_num, tx_data);
            break;
        case CAN_EPS_CTRL:
            process_eps_ctrl_tx_msg(field_num);
        default:
            return;
    }
}

void print_next_tx_msg(void) {
    if (!print_can_msgs) {
        return;
    }

    uint8_t tx_msg[8] = { 0x00 };
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (queue_empty(&can_tx_msg_queue)) {
            return;
        }
        peek_queue(&can_tx_msg_queue, tx_msg);
    }

    print("CAN TX: ");
    print_bytes(tx_msg, 8);
}

void print_next_rx_msg(void) {
    if (!print_can_msgs) {
        return;
    }

    uint8_t rx_msg[8] = { 0x00 };
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (queue_empty(&can_rx_msg_queue)) {
            return;
        }
        peek_queue(&can_rx_msg_queue, rx_msg);
    }

    print("CAN RX: ");
    print_bytes(rx_msg, 8);
}




void get_status_fn(void) {
    print("Restart count: %lu\n", restart_count);
    print("Restart reason: %lu\n", restart_reason);
    print("Uptime: %lu s\n", uptime_s);
}

void req_eps_hk_fn(void) {
    enqueue_rx_msg(CAN_EPS_HK, 0, 0);
}

void heater_1_low_fn(void) {
    enqueue_rx_msg(CAN_EPS_CTRL, CAN_EPS_CTRL_SET_HEAT1_SHAD_SP,
        heater_setpoint_to_dac_raw_data(0));
    enqueue_rx_msg(CAN_EPS_CTRL, CAN_EPS_CTRL_SET_HEAT1_SUN_SP,
        heater_setpoint_to_dac_raw_data(-20));
}

void heater_1_high_fn(void) {
    enqueue_rx_msg(CAN_EPS_CTRL, CAN_EPS_CTRL_SET_HEAT1_SHAD_SP,
        heater_setpoint_to_dac_raw_data(100));
    enqueue_rx_msg(CAN_EPS_CTRL, CAN_EPS_CTRL_SET_HEAT1_SUN_SP,
        heater_setpoint_to_dac_raw_data(65));
}

void heater_2_low_fn(void) {
    enqueue_rx_msg(CAN_EPS_CTRL, CAN_EPS_CTRL_SET_HEAT2_SHAD_SP,
        heater_setpoint_to_dac_raw_data(0));
    enqueue_rx_msg(CAN_EPS_CTRL, CAN_EPS_CTRL_SET_HEAT2_SUN_SP,
        heater_setpoint_to_dac_raw_data(-20));
}

void heater_2_high_fn(void) {
    enqueue_rx_msg(CAN_EPS_CTRL, CAN_EPS_CTRL_SET_HEAT2_SHAD_SP,
        heater_setpoint_to_dac_raw_data(100));
    enqueue_rx_msg(CAN_EPS_CTRL, CAN_EPS_CTRL_SET_HEAT2_SUN_SP,
        heater_setpoint_to_dac_raw_data(65));
}

void reset_fn(void) {
    print("Intentionally resetting...\n");
    enqueue_rx_msg(CAN_EPS_CTRL, CAN_EPS_CTRL_RESET, 0);
}




void print_cmds(void) {
    for (uint8_t i = 0; i < all_cmds_len; i++) {
        print("%u: %s\n", i, all_cmds[i].description);
    }
}

uint8_t uart_cb(const uint8_t* data, uint8_t len) {
    if (len == 0) {
        return 0;
    }

    // Print the typed character
    print("%c\n", data[0]);

    // Check for printing the help menu
    if (data[0] == 'h') {
        print_cmds();
    }

    // Check for a valid command number
    else if ('0' <= data[0] && data[0] < '0' + all_cmds_len) {
        // Enqueue the selected command
        uint8_t i = data[0] - '0';
        all_cmds[i].fn();
    }

    else {
        print("Invalid command\n");
    }

    // Processed 1 character
    return 1;
}

int main(void) {
    WDT_OFF();
    WDT_ENABLE_SYS_RESET(WDTO_8S);

    init_eps();
    // set_uart_baud_rate(UART_BAUD_115200);

    print("\n\n\nStarting commands test\n\n");

    // Change these as necessary for testing
    sim_obc = false;
    com_timeout_period_s = 600;
    hb_ping_period_s = 20;
    disable_hb = false;
    print_can_msgs = true;

    print("sim_obc = %u\n", sim_obc);
    print("com_timeout_period_s = %lu\n", com_timeout_period_s);
    print("hb_ping_period_s = %lu\n", hb_ping_period_s);
    print("disable_hb = %u\n", disable_hb);
    print("print_can_msgs = %u\n", print_can_msgs);

    // Initialize heartbeat separately so we have the option to disable it for debugging
    if (!disable_hb) {
        init_hb(HB_EPS);
    }

    print("At any time, press h to show the command menu\n");
    print_cmds();
    set_uart_rx_cb(uart_cb);

    while(1) {
        WDT_ENABLE_SYS_RESET(WDTO_8S);

        if (!disable_hb) {
            run_hb();
        }

        // Run the shunt algorithm and check if the state changed
        bool are_shunts_on_saved = are_shunts_on;
        control_shunts();
        if (are_shunts_on != are_shunts_on_saved) {
            print("Shunts changed\n");
            if (are_shunts_on) {
                print("Shunts ON (charging OFF)\n");
            } else {
                print("Shunts OFF (charging ON)\n");
            }
        }

        // CAN TX
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            print_next_tx_msg();
            if (sim_obc) {
                sim_send_next_tx_msg();
            } else {
                send_next_tx_msg();
            }
        }

        // CAN RX
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            print_next_rx_msg();
            process_next_rx_msg();
        }
    }

    return 0;
}

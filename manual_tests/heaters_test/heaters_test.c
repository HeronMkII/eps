/*
Test heaters by typing in commands over UART to manually turn on and off heaters.
Also display thermistor measurements.
*/

#include <stdbool.h>

#include <adc/adc.h>
#include <uart/uart.h>

#include "../../src/devices.h"
#include "../../src/heaters.h"
#include "../../src/measurements.h"


// Callback function signature to run a command
typedef void(*uart_cmd_fn_t)(void);

// UART-activated command
typedef struct {
    char* description;
    uart_cmd_fn_t fn;
} uart_cmd_t;

void read_data_fn(void);
void set_heater_1_setpoint_fn(void);
void set_heater_2_setpoint_fn(void);
void turn_heater_1_on_fn(void);
void turn_heater_1_off_fn(void);
void turn_heater_2_on_fn(void);
void turn_heater_2_off_fn(void);

volatile bool entering_num = false;
volatile uint8_t entering_num_heater = 0;   // 1 or 2


// All possible commands
uart_cmd_t all_cmds[] = {
    {
        .description = "Read data",
        .fn = read_data_fn
    },
    {
        .description = "Set heater 1 setpoint",
        .fn = set_heater_1_setpoint_fn
    },
    {
        .description = "Set heater 2 setpoint",
        .fn = set_heater_2_setpoint_fn
    }
    // {
    //     .description = "Turn heater 1 on",
    //     .fn = turn_heater_1_on_fn
    // },
    // {
    //     .description = "Turn heater 1 off",
    //     .fn = turn_heater_1_off_fn
    // },
    // {
    //     .description = "Turn heater 2 on",
    //     .fn = turn_heater_2_on_fn
    // },
    // {
    //     .description = "Turn heater 2 off",
    //     .fn = turn_heater_2_off_fn
    // }
};
// Length of array
const uint8_t all_cmds_len = sizeof(all_cmds) / sizeof(all_cmds[0]);


void set_heater_1(double temp) {
    set_heater_1_raw_shadow_setpoint(heater_setpoint_temp_to_raw(temp));
    print("Set heater 1 setpoint (DAC A) = %.1f C\n", temp);
}

void set_heater_2(double temp) {
    set_heater_2_raw_shadow_setpoint(heater_setpoint_temp_to_raw(temp));
    print("Set heater 2 setpoint (DAC B) = %.1f C\n", temp);
}


void read_voltage(uint8_t channel) {
    fetch_adc_channel(&adc, channel);
    uint16_t raw_data = read_adc_channel(&adc, channel);
    double voltage = adc_raw_data_to_eps_vol(raw_data);
    print(", %.6f", voltage);
}

// read_adc_current function definition now in heaters.c and returns the current value
void read_current(uint8_t channel) {			
    double current = read_eps_cur(channel);
    print(", %.6f", current);		
}

void read_therm(uint8_t channel) {
    fetch_adc_channel(&adc, channel);
    uint16_t raw_data = read_adc_channel(&adc, channel);
    double temp = adc_raw_data_to_therm_temp(raw_data);
    print(", %.6f", temp);
}

void read_setpoint(uint16_t raw_voltage) {
    print(", %.6f", therm_res_to_temp(therm_vol_to_res(
        dac_raw_data_to_vol(raw_voltage))));
}

void read_data_fn(void) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        read_voltage(MEAS_BB_VOUT);
        read_current(MEAS_BB_IOUT);
        read_current(MEAS_NEG_Y_IOUT);
        read_current(MEAS_POS_X_IOUT);
        read_current(MEAS_POS_Y_IOUT);
        read_current(MEAS_NEG_X_IOUT);
        read_therm(MEAS_THERM_1);
        read_therm(MEAS_THERM_2);
        read_voltage(MEAS_PACK_VOUT);

        // Use a different conversion formula for battery current (bipolar operation)
        // TODO - change conversion in lib-common
        uint8_t channel = MEAS_PACK_IOUT;
        fetch_adc_channel(&adc, channel);
        uint16_t raw_data = read_adc_channel(&adc, channel);
        double current = adc_raw_data_to_eps_cur(raw_data) - 2.5;
        print(", %.6f", current);

        read_current(MEAS_BT_IOUT);
        read_voltage(MEAS_BT_VOUT);
        read_setpoint(dac.raw_voltage_a);
        read_setpoint(dac.raw_voltage_b);
        print("\n");
    }
}

void set_heater_1_setpoint_fn(void) {
    print("Enter a number of the format ##.#\n");
    entering_num_heater = 1;
    entering_num = true;
}

void set_heater_2_setpoint_fn(void) {
    print("Enter a number of the format ##.#\n");
    entering_num_heater = 2;
    entering_num = true;
}

void turn_heater_1_on_fn(void) {
    set_heater_1(100);
    print("Heater 1 should be ON\n");
}

void turn_heater_1_off_fn(void) {
    set_heater_1(0);
    print("Heater 1 should be OFF\n");
}

void turn_heater_2_on_fn(void) {
    set_heater_2(100);
    print("Heater 2 should be ON\n");
}

void turn_heater_2_off_fn(void) {
    set_heater_2(0);
    print("Heater 2 should be OFF\n");
}


void print_cmds(void) {
    print("\n");
    for (uint8_t i = 0; i < all_cmds_len; i++) {
        print("%u: %s\n", i, all_cmds[i].description);
    }
}

bool is_num(char c) {
    if ('0' <= c && c <= '9') {
        return true;
    } else {
        return false;
    }
}

uint8_t char_to_num(char c) {
    return c - '0';
}

uint8_t uart_cb(const uint8_t* data, uint8_t len) {
    if (len == 0) {
        return 0;
    }

    // Print the typed character
    put_uart_char(data[len - 1]);

    if (entering_num) {
        if (len < 4) {
            return 0;
        }
        put_uart_char('\n');

        entering_num = false;

        if (!(is_num(data[0]) && is_num(data[1]) && data[2] == '.' && is_num(data[3]))) {
            print("Invalid number, must be of the form ##.#\n");
            return len;
        }

        double entered_num =
            (10.0 * char_to_num(data[0])) +
            (1.0 * char_to_num(data[1])) +
            (0.1 * char_to_num(data[3]));

        if (entering_num_heater == 1) {
            set_heater_1(entered_num);
        } else if (entering_num_heater == 2) {
            set_heater_2(entered_num);
        }

        return len;
    }

    put_uart_char('\n');

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
    init_uart();

    // Set the IMU CSn (PD0) high (because it doesn't have a pullup resistor)
    // so it doesn't interfere with MISO line
    init_cs(PD0, &DDRD);
    set_cs_high(PD0, &PORTD);

    init_spi();
    init_dac(&dac);
    init_adc(&adc);

    init_heaters();

    print("\n\n\nStarting test\n\n");
    print("DAC Raw Voltage A: %.4x = %f c\n", dac.raw_voltage_a, adc_raw_data_to_therm_temp(dac.raw_voltage_a));
    print("DAC Raw Voltage B: %.4x = %f c\n", dac.raw_voltage_b, adc_raw_data_to_therm_temp(dac.raw_voltage_b));

    print("\nAt any time, press h to show the command menu\n");
    print_cmds();
    set_uart_rx_cb(uart_cb);

    print(", BB Vol (V), BB Cur (A), -Y Cur (A), +X Cur (A), +Y Cur (A)");
    print(", -X Cur (A), Temp 1 (C), Temp 2 (C), Bat Vol (V), Bat Cur (A)");
    print(", BT Cur (A), BT Vol (V), Setpoint 1 (C), Setpoint 2 (C)");
    print("\n");

    while (1) {
        read_data_fn();
        _delay_ms(5000);
    }

    return 0;
}

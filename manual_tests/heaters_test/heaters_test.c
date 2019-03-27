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

void read_thermistor_data_fn(void);
void set_heater_1_arbitrary_fn(void);
void set_heater_2_arbitrary_fn(void);
void turn_heater_1_on_fn(void);
void turn_heater_1_off_fn(void);
void set_heater_1_mid_fn(void);
void turn_heater_2_on_fn(void);
void turn_heater_2_off_fn(void);
void set_heater_2_mid_fn(void);

volatile bool entering_num = false;
volatile bool entered_num_valid = false;
volatile double entered_num = 0;


// All possible commands
uart_cmd_t all_cmds[] = {
    {
        .description = "Read thermistor data",
        .fn = read_thermistor_data_fn
    },
    {
        .description = "Set heater 1 arbitrary setpoint",
        .fn = set_heater_1_arbitrary_fn
    },
    {
        .description = "Set heater 2 arbitrary setpoint",
        .fn = set_heater_2_arbitrary_fn
    },
    {
        .description = "Turn heater 1 on",
        .fn = turn_heater_1_on_fn
    },
    {
        .description = "Turn heater 1 off",
        .fn = turn_heater_1_off_fn
    },
    {
        .description = "Set heater 1 middle setpoint",
        .fn = set_heater_1_mid_fn
    },
    {
        .description = "Turn heater 2 on",
        .fn = turn_heater_2_on_fn
    },
    {
        .description = "Turn heater 2 off",
        .fn = turn_heater_2_off_fn
    },
    {
        .description = "Set heater 2 middle setpoint",
        .fn = set_heater_2_mid_fn
    }
};
// Length of array
const uint8_t all_cmds_len = sizeof(all_cmds) / sizeof(all_cmds[0]);

// Modify this array to contain the ADC channels you want to monitor
// (channels 10 and 11 are something else - motor positioning sensors)
uint8_t adc_channels[] = { MEAS_THERM_1, MEAS_THERM_2 };
// uint8_t adc_channels[] = {0, 1, 2};
// automatically calculate the length of the array
uint8_t adc_channels_len = sizeof(adc_channels) / sizeof(adc_channels[0]);


void set_heater_1(double temp) {
    set_heater_1_temp_setpoint(temp);
    print("Set heater 1 setpoint (DAC A) = %.1f C\n", temp);
}

void set_heater_2(double temp) {
    set_heater_2_temp_setpoint(temp);
    print("Set heater 2 setpoint (DAC B) = %.1f C\n", temp);
}


void read_thermistor_data_fn(void) {
    print("\n");
    print("Channel, Raw (12 bits), Voltage (V), Resistance (kohms), Temperature (C)\n");

    //Find resistance for each channel
    //only calculate it for the thermistors specified in adc_channels
    for (uint8_t i = 0; i < adc_channels_len; i++) {
        // Read ADC channel data
        uint8_t channel = adc_channels[i];
        fetch_adc_channel(&adc, channel);
        uint16_t raw_data = read_adc_channel(&adc, channel);

        double voltage = adc_raw_data_to_raw_vol(raw_data);
        //Convert adc voltage to resistance of thermistor
        double resistance = therm_vol_to_res(voltage);
        //Convert resistance to temperature of thermistor
        double temperature = adc_raw_data_to_therm_temp(raw_data);

        print("%u: 0x%.3X, %.3f, %.3f, %.3f\n", channel, raw_data, voltage, resistance, temperature);
    }
}

void set_heater_1_arbitrary_fn(void) {
    print("Enter a number of the format ##.#\n");
    entering_num = true;
    while (entering_num) {}
    if (entered_num_valid) {
        set_heater_1(entered_num);
    }
    entered_num_valid = false;
}

void set_heater_2_arbitrary_fn(void) {
    print("Enter a number of the format ##.#\n");
    entering_num = true;
    while (entering_num) {}
    if (entered_num_valid) {
        set_heater_2(entered_num);
    }
    entered_num_valid = false;
}

void turn_heater_1_on_fn(void) {
    set_heater_1(100);
    print("Heater 1 should be ON\n");
}

void turn_heater_1_off_fn(void) {
    set_heater_1(0);
    print("Heater 1 should be OFF\n");
}

void set_heater_1_mid_fn(void) {
    set_heater_1(28);
}

void turn_heater_2_on_fn(void) {
    set_heater_2(100);
    print("Heater 2 should be ON\n");
}

void turn_heater_2_off_fn(void) {
    set_heater_2(0);
    print("Heater 2 should be OFF\n");
}

void set_heater_2_mid_fn(void) {
    set_heater_2(28);
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

    if (entering_num) {
        if (len < 4) {
            return 0;
        }
        entering_num = false;
        if (is_num(data[0]) && is_num(data[1]) && data[2] == '.' && is_num(data[3])) {
            entered_num =
                (10.0 * char_to_num(data[0])) +
                (1.0 * char_to_num(data[1])) +
                (0.1 * char_to_num(data[3]));
            entered_num_valid = true;
            print("Got number %.1f\n", entered_num);
        } else {
            entered_num = 0.0;
            entered_num_valid = false;
            print("Invalid number, must be of the form ##.#\n");
        }

        return len;
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
        print_cmds();
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

    print("\n\n\nStarting test\n\n");

    turn_heater_1_off_fn();
    turn_heater_2_off_fn();

    print("\nAt any time, press h to show the command menu\n");
    print_cmds();
    set_uart_rx_cb(uart_cb);

    while (1) {}

    return 0;
}

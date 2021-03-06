/*
Test heaters by typing in commands over UART to manually turn on and off heaters.
Also display thermistor measurements.
*/

#include <stdbool.h>

#include <adc/adc.h>
#include <uart/uart.h>

#include "../../src/devices.h"
#include "../../src/heaters.h"


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
    set_raw_heater_setpoint(&heater_1_shadow_setpoint, heater_setpoint_to_dac_raw_data(temp));
    set_raw_heater_setpoint(&heater_1_sun_setpoint, heater_setpoint_to_dac_raw_data(temp));
    print("Set heater 1 setpoint (DAC A) = %.1f C\n", temp);
}

void set_heater_2(double temp) {
    set_raw_heater_setpoint(&heater_2_shadow_setpoint, heater_setpoint_to_dac_raw_data(temp));
    set_raw_heater_setpoint(&heater_2_sun_setpoint, heater_setpoint_to_dac_raw_data(temp));
    print("Set heater 2 setpoint (DAC B) = %.1f C\n", temp);
}


void read_voltage(uint8_t channel) {
    fetch_adc_channel(&adc, channel);
    uint16_t raw_data = read_adc_channel(&adc, channel);
    double voltage = adc_raw_to_circ_vol(raw_data, ADC_VOL_SENSE_LOW_RES, ADC_VOL_SENSE_HIGH_RES);
    print(", %.6f", voltage);
}

double read_eps_cur(uint8_t channel) {
    uint16_t raw = fetch_and_read_adc_channel(&adc, channel);
    return adc_raw_to_circ_cur(raw, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF);
}

void read_current(uint8_t channel) {			
    double current = read_eps_cur(channel);
    print(", %.6f", current);		
}

void read_therm(uint8_t channel) {
    fetch_adc_channel(&adc, channel);
    uint16_t raw_data = read_adc_channel(&adc, channel);
    double temp = adc_raw_to_therm_temp(raw_data);
    print(", %.6f", temp);
}

void read_setpoint(uint16_t raw_voltage) {
    print(", %.6f", therm_res_to_temp(therm_vol_to_res(
        dac_raw_data_to_vol(raw_voltage))));
}

void read_data_fn(void) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        read_setpoint(dac.raw_voltage_a);
        read_setpoint(dac.raw_voltage_b);
        read_therm(ADC_THM_BATT1);
        read_therm(ADC_THM_BATT2);
        read_current(ADC_IMON_PACK);
        read_current(ADC_IMON_5V);
        read_therm(ADC_THM_PAY_CONN);
        read_therm(ADC_THM_3V3_TOP);
        read_therm(ADC_THM_5V_TOP);
        read_current(ADC_IMON_Y_MINUS);
        read_current(ADC_IMON_X_PLUS);
        read_current(ADC_IMON_Y_PLUS);
        read_current(ADC_IMON_X_MINUS);
        read_current(ADC_IMON_3V3);
        read_voltage(ADC_VMON_PACK);
        read_voltage(ADC_VMON_3V3);
        read_voltage(ADC_VMON_5V);
        read_current(ADC_IMON_PAY_LIM);
        
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
    print("DAC Raw Voltage A: %.4x = %f c\n", dac.raw_voltage_a, adc_raw_to_therm_temp(dac.raw_voltage_a));
    print("DAC Raw Voltage B: %.4x = %f c\n", dac.raw_voltage_b, adc_raw_to_therm_temp(dac.raw_voltage_b));

    print("\nAt any time, press h to show the command menu\n");
    print_cmds();
    set_uart_rx_cb(uart_cb);

    print(", Setpoint 1 (C), Setpoint 2 (C), Temp 1 (C), Temp 2 (C)");
    print(", Bat Cur (A), 5V Cur (A), PAY Con Temp (C), 3V3 Temp (C), 5V Temp (C)");
    print(", -Y Cur (A), +X Cur (A), +Y Cur (A), -X Cur (A)");
    print(", 3V3 Cur (A), Bat Vol (V), 3V3 Vol (V)");
    print(", 5V Vol (V), PAY Vol (V)");
    print("\n");

    while (1) {
        read_data_fn();
        _delay_ms(5000);
    }

    return 0;
}

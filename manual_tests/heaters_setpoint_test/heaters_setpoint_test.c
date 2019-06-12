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


void read_current(char* str, uint8_t channel) {			
    double current = read_eps_cur(channel);
    print("%s: %.6f\n", str, current);		
}

void read_therm(char* str, uint8_t channel) {
    fetch_adc_channel(&adc, channel);
    uint16_t raw_data = read_adc_channel(&adc, channel);
    double temp = adc_raw_data_to_therm_temp(raw_data);
    print("%s: %.6f\n", str, temp);
}

// This is for an "active" or "non-active" setpoint
void read_setpoint(char* str, uint16_t raw_voltage) {
    print("%s: %.6f\n", str, heater_setpoint_raw_to_temp(raw_voltage));
}

void read_cur_thresh(char* str, uint16_t raw_cur) {
    print("%s: %.6f\n", str, heater_cur_thresh_raw_to_cur(raw_cur));
}

void read_data(void) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        read_setpoint("Header 1 shadow (C)", heater_1_shadow_setpoint.raw);
        read_setpoint("Header 2 shadow (C)", heater_2_shadow_setpoint.raw);
        read_setpoint("Header 1 sun (C)", heater_1_sun_setpoint.raw);
        read_setpoint("Header 2 sun (C)", heater_2_sun_setpoint.raw);
        read_cur_thresh("Upper current threshold (A)", heater_sun_cur_thresh_upper.raw);
        read_cur_thresh("Lower current threshold (A)", heater_sun_cur_thresh_lower.raw);
        read_current("-Y Cur (A)", MEAS_NEG_Y_IOUT);
        read_current("+X Cur (A)", MEAS_POS_X_IOUT);
        read_current("+Y Cur (A)", MEAS_POS_Y_IOUT);
        read_current("-X Cur (A)", MEAS_NEG_X_IOUT);
        read_therm("Temp 1 (C)", MEAS_THERM_1);
        read_therm("Temp 2 (C)", MEAS_THERM_2);
        read_setpoint("Setpoint 1 (C)", dac.raw_voltage_a);
        read_setpoint("Setpoint 2 (C)", dac.raw_voltage_b);
        print("\n");
    }
}


void set_setpoints(float shadow, float sun) {
    set_raw_heater_setpoint(&heater_1_shadow_setpoint,
        heater_setpoint_temp_to_raw(shadow));
    set_raw_heater_setpoint(&heater_2_shadow_setpoint,
        heater_setpoint_temp_to_raw(shadow));
    set_raw_heater_setpoint(&heater_1_sun_setpoint,
        heater_setpoint_temp_to_raw(sun));
    set_raw_heater_setpoint(&heater_2_sun_setpoint,
        heater_setpoint_temp_to_raw(sun));
}

void set_cur_threshes(float upper, float lower) {
    set_raw_heater_cur_thresh(&heater_sun_cur_thresh_upper,
        heater_cur_thresh_cur_to_raw(upper));
    set_raw_heater_cur_thresh(&heater_sun_cur_thresh_lower,
        heater_cur_thresh_cur_to_raw(lower));
}

void print_cmds(void) {
    print("1. Setpoints 20/5\n");
    print("2. Setpoints 30/10\n");
    print("3. Thresholds 1/0.95\n");
    print("4. Thresholds 0.3/0.2\n");
}

uint8_t uart_cb(const uint8_t* buf, uint8_t data) {
    switch (buf[0]) {
        case 'h':
            print_cmds();
            break;
        case '1':
            set_setpoints(20, 5);
            print("Changed setpoints to 20/5\n");
            break;
        case '2':
            set_setpoints(30, 10);
            print("Changed setpoints to 30/10\n");
            break;
        case '3':
            set_cur_threshes(1, 0.95);
            print("Changed current thresholds to 1/0.95\n");
            break;
        case '4':
            set_cur_threshes(0.3, 0.2);
            print("Changed current thresholds to 0.3/0.2\n");
            break;
        default:
            print("Invalid command\n");
            break;
    }

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

    init_heaters();

    set_uart_rx_cb(uart_cb);
    print_cmds();
    print("Press h to list commands\n");

    while (1) {
        read_data();
        control_heater_mode();
        print("\n");
        _delay_ms(5000);
    }

    return 0;
}

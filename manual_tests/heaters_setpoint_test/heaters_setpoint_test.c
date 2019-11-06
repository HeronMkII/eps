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


// This is for an "active" or "non-active" setpoint
void read_setpoint(char* str, uint16_t raw_voltage) {
    print("%s: %.6f C\n", str, dac_raw_data_to_heater_setpoint(raw_voltage));
}

void read_cur_thresh(char* str, uint16_t raw_cur) {
    print("%s: %.6f A\n", str, adc_raw_to_circ_cur(raw_cur, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF));
}

void read_current(char* str, uint8_t channel) {			
    double current = read_eps_cur(channel);
    print("%s: %.6f A\n", str, current);		
}

void read_current_sum(char* str) {
    double current =
        read_eps_cur(MEAS_NEG_Y_IOUT) +
        read_eps_cur(MEAS_POS_X_IOUT) +
        read_eps_cur(MEAS_POS_Y_IOUT) +
        read_eps_cur(MEAS_NEG_X_IOUT);
    print("%s: %.6f A\n", str, current);
}

void read_therm(char* str, uint8_t channel) {
    fetch_adc_channel(&adc, channel);
    uint16_t raw_data = read_adc_channel(&adc, channel);
    double temp = adc_raw_to_therm_temp(raw_data);
    print("%s: %.6f C\n", str, temp);
}

void read_data(void) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        read_setpoint("Header 1 shadow", heater_1_shadow_setpoint.raw);
        read_setpoint("Header 2 shadow", heater_2_shadow_setpoint.raw);
        read_setpoint("Header 1 sun", heater_1_sun_setpoint.raw);
        read_setpoint("Header 2 sun", heater_2_sun_setpoint.raw);
        read_cur_thresh("Upper current threshold", heater_sun_cur_thresh_upper.raw);
        read_cur_thresh("Lower current threshold", heater_sun_cur_thresh_lower.raw);
        read_current("-Y Cur", MEAS_NEG_Y_IOUT);
        read_current("+X Cur", MEAS_POS_X_IOUT);
        read_current("+Y Cur", MEAS_POS_Y_IOUT);
        read_current("-X Cur", MEAS_NEG_X_IOUT);
        read_current_sum("Sum Cur");
        read_therm("Temp 1", MEAS_THERM_1);
        read_therm("Temp 2", MEAS_THERM_2);
        read_setpoint("Setpoint 1", dac.raw_voltage_a);
        read_setpoint("Setpoint 2", dac.raw_voltage_b);
        print("\n");
    }
}


void set_setpoints(float shadow, float sun) {
    set_raw_heater_setpoint(&heater_1_shadow_setpoint,
        heater_setpoint_to_dac_raw_data(shadow));
    set_raw_heater_setpoint(&heater_2_shadow_setpoint,
        heater_setpoint_to_dac_raw_data(shadow));
    set_raw_heater_setpoint(&heater_1_sun_setpoint,
        heater_setpoint_to_dac_raw_data(sun));
    set_raw_heater_setpoint(&heater_2_sun_setpoint,
        heater_setpoint_to_dac_raw_data(sun));
}

void set_cur_threshes(float upper, float lower) {
    set_raw_heater_cur_thresh(&heater_sun_cur_thresh_upper,
        adc_circ_cur_to_raw(upper, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF));
    set_raw_heater_cur_thresh(&heater_sun_cur_thresh_lower,
        adc_circ_cur_to_raw(lower, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF));
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
    print("Press h to list commands\n\n");

    while (1) {
        read_data();
        control_heater_mode();
        _delay_ms(3000);
    }

    return 0;
}

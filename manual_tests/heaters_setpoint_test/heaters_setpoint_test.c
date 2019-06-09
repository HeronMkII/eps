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

void read_data(void) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        read_current(MEAS_NEG_Y_IOUT);
        read_current(MEAS_POS_X_IOUT);
        read_current(MEAS_POS_Y_IOUT);
        read_current(MEAS_NEG_X_IOUT);
        read_therm(MEAS_THERM_1);
        read_therm(MEAS_THERM_2);
        read_setpoint(dac.raw_voltage_a);
        read_setpoint(dac.raw_voltage_b);
    }
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

    set_heater_1_raw_shadow_setpoint(heater_setpoint_temp_to_raw(HEATER_1_STANDARD_SHADOW_SETPOINT));
    set_heater_2_raw_shadow_setpoint(heater_setpoint_temp_to_raw(HEATER_2_STANDARD_SHADOW_SETPOINT));
    set_heater_1_raw_sun_setpoint(heater_setpoint_temp_to_raw(HEATER_1_STANDARD_SUN_SETPOINT));
    set_heater_2_raw_sun_setpoint(heater_setpoint_temp_to_raw(HEATER_2_STANDARD_SUN_SETPOINT));

    print("-Y Cur (A), +X Cur (A), +Y Cur (A), -X Cur (A)");
    print(", Temp 1 (C), Temp 2 (C), Setpoint 1 (C), Setpoint 2 (C)");
    print("\n");

    while (1) {
        read_data();
        control_heater_mode();
        print("\n");
        _delay_ms(5000);
    }

    return 0;
}

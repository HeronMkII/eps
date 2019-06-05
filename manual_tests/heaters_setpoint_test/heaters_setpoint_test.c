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


void read_data_fn(void);

void read_voltage(uint8_t channel) {
    fetch_adc_channel(&adc, channel);
    uint16_t raw_data = read_adc_channel(&adc, channel);
    double voltage = adc_raw_data_to_eps_vol(raw_data);
    print(", %.6f", voltage);
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
        read_current(MEAS_BB_IOUT);
        read_current(MEAS_NEG_Y_IOUT);
        read_current(MEAS_POS_X_IOUT);
        read_current(MEAS_POS_Y_IOUT);
        read_current(MEAS_NEG_X_IOUT);

        // Use a different conversion formula for battery current (bipolar operation)
        // TODO - change conversion in lib-common
        uint8_t channel = MEAS_PACK_IOUT;
        fetch_adc_channel(&adc, channel);
        uint16_t raw_data = read_adc_channel(&adc, channel);
        double current = adc_raw_data_to_eps_cur(raw_data) - 2.5;
        print(", %.6f", current);

        read_current(MEAS_BT_IOUT);
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

    print("BB Cur (A), -Y Cur (A), +X Cur (A), +Y Cur (A), -X Cur (A), Temp 1 (C), Temp 2 (C), Bat Cur (A), BT Cur (A)");
    print(", Before Setpoint 1 (C), Before Setpoint 2 (C), After Setpoint 1 (C), After Setpoint 2 (C)");
    print("\n");

    while (1) {
        read_data_fn();
        sunorshadow_setpoint();
        print("\n");
        _delay_ms(5000);
    }

    return 0;
}

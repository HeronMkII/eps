#include <uart/uart.h>
#include <adc/adc.h>
#include <adc/eps.h>
#include <spi/spi.h>
#include <conversions/conversions.h>

#include "../../src/devices.h"
#include "../../src/measurements.h"

void read_voltage(uint8_t channel) {
    fetch_channel(&adc, channel);
    uint16_t raw_data = read_channel(&adc, channel);
    double raw_voltage = adc_raw_data_to_raw_vol(raw_data);
    double voltage = adc_raw_vol_to_eps_vol(raw_voltage);
    print("Channel: %u, Raw Data: 0x%04x, Raw Voltage: %.6f V, Voltage: %.6f V\n",
            channel, raw_data, raw_voltage, voltage);
}

void read_current(uint8_t channel) {
    fetch_channel(&adc, channel);
    uint16_t raw_data = read_channel(&adc, channel);
    double raw_voltage = adc_raw_data_to_raw_vol(raw_data);
    double current = adc_raw_vol_to_eps_cur(raw_voltage);
    print("Channel: %u, Raw Data: 0x%04x, Raw Voltage: %.6f V, Current: %.6f A\n",
            channel, raw_data, raw_voltage, current);
}

void read_therm(uint8_t channel) {
    fetch_channel(&adc, channel);
    uint16_t raw_data = read_channel(&adc, channel);
    double raw_voltage = adc_raw_data_to_raw_vol(raw_data);
    double temp = adc_raw_data_to_therm_temp(raw_data);
    print("Channel: %u, Raw Data: 0x%04x, Raw Voltage: %.6f V, Temperature: %.6f C\n",
            channel, raw_data, raw_voltage, temp);
}


// This test reads the raw data and voltages on each ADC channel
// It converts the raw voltages to actual voltages and currents
int main(void) {
    init_uart();
    print("\n\nUART initialized\n");

    init_spi();
    print("SPI Initialized\n");

    init_adc(&adc);
    print("ADC Initialized\n");

    print("\nStarting test\n\n");

    while(1) {
        print("BB VOUT\n");
        read_voltage(MEAS_BB_VOUT);
        print("BB IOUT\n");
        read_current(MEAS_BB_IOUT);
        print("NEG Y IOUT\n");
        read_current(MEAS_NEG_Y_IOUT);
        print("POS X IOUT\n");
        read_current(MEAS_POS_X_IOUT);
        print("POS Y IOUT\n");
        read_current(MEAS_POS_Y_IOUT);
        print("NEG X IOUT\n");
        read_current(MEAS_NEG_X_IOUT);
        print("THERM 1\n");
        read_therm(MEAS_THERM_1);
        print("THERM 2\n");
        read_therm(MEAS_THERM_2);
        print("PACK VOUT\n");
        read_voltage(MEAS_PACK_VOUT);
        print("PACK IOUT\n");
        read_current(MEAS_PACK_IOUT);
        print("BT IOUT\n");
        read_current(MEAS_BT_IOUT);
        print("BT VOUT\n");
        read_voltage(MEAS_BT_VOUT);

        print("\n");
        _delay_ms(5000);
    }
}

#include <uart/uart.h>
#include <adc/adc.h>
#include <spi/spi.h>
#include <conversions/conversions.h>

#include "../../src/devices.h"
#include "../../src/measurements.h"

void read_voltage(char* name, uint8_t channel) {
    fetch_adc_channel(&adc, channel);
    uint16_t raw_data = read_adc_channel(&adc, channel);
    double raw_voltage = adc_raw_data_to_raw_vol(raw_data);
    double voltage = adc_raw_data_to_eps_vol(raw_data);
    print("%s, %u, 0x%04x, %.6f V, %.6f V\n",
            name, channel, raw_data, raw_voltage, voltage);
}

void read_current(char* name, uint8_t channel) {
    fetch_adc_channel(&adc, channel);
    uint16_t raw_data = read_adc_channel(&adc, channel);
    double raw_voltage = adc_raw_data_to_raw_vol(raw_data);
    double current = adc_raw_data_to_eps_cur(raw_data);
    print("%s, %u, 0x%04x, %.6f V, %.6f A\n",
            name, channel, raw_data, raw_voltage, current);
}

void read_therm(char* name, uint8_t channel) {
    fetch_adc_channel(&adc, channel);
    uint16_t raw_data = read_adc_channel(&adc, channel);
    double raw_voltage = adc_raw_data_to_raw_vol(raw_data);
    double temp = adc_raw_data_to_therm_temp(raw_data);
    print("%s, %u, 0x%04x, %.6f V, %.6f C\n",
            name, channel, raw_data, raw_voltage, temp);
}


// This test reads the raw data and voltages on each ADC channel
// It converts the raw voltages to actual voltages and currents
int main(void) {
    init_uart();
    print("\n\nUART initialized\n");

    // Set the IMU CSn (PD0) high (because it doesn't have a pullup resistor)
    // so it doesn't interfere with the ADC's output on the MISO line
    init_cs(PD0, &DDRD);
    set_cs_high(PD0, &PORTD);

    init_spi();
    print("SPI Initialized\n");

    init_adc(&adc);
    print("ADC Initialized\n");

    print("\nStarting test\n\n");

    while(1) {
        print("Name, Channel, Raw Data, Raw Voltage, Converted Data\n");
        read_voltage("BB VOUT", MEAS_BB_VOUT);
        read_current("BB IOUT", MEAS_BB_IOUT);
        read_current("-Y IOUT", MEAS_NEG_Y_IOUT);
        read_current("+X IOUT", MEAS_POS_X_IOUT);
        read_current("-Y IOUT", MEAS_POS_Y_IOUT);
        read_current("-X IOUT", MEAS_NEG_X_IOUT);
        read_therm("THERM 1", MEAS_THERM_1);
        read_therm("THERM 2", MEAS_THERM_2);
        read_voltage("PACK VOUT", MEAS_PACK_VOUT);
        read_current("PACK IOUT", MEAS_PACK_IOUT);
        read_current("BT IOUT", MEAS_BT_IOUT);
        read_voltage("BT VOUT", MEAS_BT_VOUT);

        print("\n");
        _delay_ms(5000);
    }
}

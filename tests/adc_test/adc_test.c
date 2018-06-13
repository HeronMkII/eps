#ifndef F_CPU
#define F_CPU 8000000L
#endif

#include <uart/uart.h>
#include <adc/adc.h>
#include <spi/spi.h>
#include <util/delay.h>

void read_voltage(uint8_t channel) {
    uint16_t raw_data = adc_read_channel_raw_data(channel);
    double raw_voltage = adc_convert_raw_data_to_raw_voltage(raw_data);
    double voltage = adc_eps_convert_raw_voltage_to_voltage(raw_voltage, channel);
    print("\nChannel: %u, Raw Data: 0x%04x, Raw Voltage: %lf, Voltage: %lf\n", channel, raw_data, raw_voltage, voltage);
}

void read_current(uint8_t channel) {
    uint16_t raw_data = adc_read_channel_raw_data(channel);
    double raw_voltage = adc_convert_raw_data_to_raw_voltage(raw_data);
    double current = adc_eps_convert_raw_voltage_to_current(raw_voltage, channel);
    print("Channel: %u, Raw Data: 0x%04x, Raw Voltage: %lf, Current: %lf\n\n", channel, raw_data, raw_voltage, current);
}

// This test reads the raw data and voltages on each ADC channel
// It converts the raw voltages to actual voltages and currents
int main(void) {
    init_uart();
    print("\n\nUART initialized\n");

    adc_init_constants(ADC_EPS);
    adc_init();
    print("ADC initialized\n\n\n");

    while(1){
        print("ADC_EPS_BB_VOUT_CH\n");
        read_voltage(ADC_EPS_BB_VOUT_CH);
        print("ADC_EPS_BT_VOUT_CH\n");
        read_voltage(ADC_EPS_BT_VOUT_CH);
        print("ADC_EPS_BATT_VPOS_CH\n");
        read_voltage(ADC_EPS_BATT_VPOS_CH);
        print("ADC_EPS_BATT_VNEG_CH\n");
        read_voltage(ADC_EPS_BATT_VNEG_CH);

        print("ADC_EPS_BB1_IOUT_CH\n");
        read_current(ADC_EPS_BB1_IOUT_CH);
        print("ADC_EPS_BB2_IOUT_CH\n");
        read_current(ADC_EPS_BB2_IOUT_CH);
        print("ADC_EPS_BT1_IOUT_CH\n");
        read_current(ADC_EPS_BT1_IOUT_CH);
        print("ADC_EPS_BATT_IOUT_CH\n");
        read_current(ADC_EPS_BATT_IOUT_CH);

        print("ADC_EPS_PV_POS_X_IOUT_CH\n");
        read_current(ADC_EPS_PV_POS_X_IOUT_CH);
        print("ADC_EPS_PV_NEG_X_IOUT_CH\n");
        read_current(ADC_EPS_PV_NEG_X_IOUT_CH);
        print("ADC_EPS_PV_POS_Y_IOUT_CH\n");
        read_current(ADC_EPS_PV_POS_Y_IOUT_CH);
        print("ADC_EPS_PV_NEG_Y_IOUT_CH\n");
        read_current(ADC_EPS_PV_NEG_Y_IOUT_CH);

        print("\n\n");
        _delay_ms(5000);
    }
}

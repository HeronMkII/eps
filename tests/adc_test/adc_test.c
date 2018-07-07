#ifndef F_CPU
#define F_CPU 8000000L
#endif

#include <uart/uart.h>
#include <adc/adc.h>
#include <spi/spi.h>
#include <util/delay.h>
#include <conversions/conversions.h>


void read_voltage(uint8_t channel) {
    uint16_t raw_data = adc_read_channel_raw_data(channel);
    double raw_voltage = adc_convert_raw_data_to_raw_voltage(raw_data);
    double voltage = adc_eps_convert_raw_voltage_to_voltage(raw_voltage);
    print("Channel: %u, Raw Data: 0x%04x, Raw Voltage: %ld mV, Voltage: %ld mV\n\n",
            channel, raw_data, (int32_t) (raw_voltage * 1000.0), (int32_t) (voltage * 1000.0));
}

void read_current(uint8_t channel) {
    uint16_t raw_data = adc_read_channel_raw_data(channel);
    double raw_voltage = adc_convert_raw_data_to_raw_voltage(raw_data);
    double current = adc_eps_convert_raw_voltage_to_current(raw_voltage);
    print("Channel: %u, Raw Data: 0x%04x, Raw Voltage: %ld mV, Current: %ld mA\n\n",
            channel, raw_data, (int32_t) (raw_voltage * 1000.0), (int32_t) (current * 1000.0));
}


// This test reads the raw data and voltages on each ADC channel
// It converts the raw voltages to actual voltages and currents
int main(void) {
    init_uart();
    print("\n\nUART initialized\n");

    init_spi();
    print("SPI Initialized\n");

    adc_init(ADC_EPS);
    print("ADC Initialized\n\n");

    print("Starting test\n\n");

    while(1){
        print("ADC_EPS_BB_VOUT_CH\n");
        read_voltage(ADC_EPS_BB_VOUT_CH);
        print("ADC_EPS_BT_VOUT_CH\n");
        read_voltage(ADC_EPS_BT_VOUT_CH);
        print("ADC_EPS_BATT_VPOS_CH\n");
        read_voltage(ADC_EPS_BATT_VPOS_CH);
        print("ADC_EPS_BATT_VNEG_CH\n");
        read_voltage(ADC_EPS_BATT_VNEG_CH);

        // print("ADC_EPS_BB1_IOUT_CH\n");
        // read_current(ADC_EPS_BB1_IOUT_CH);
        print("ADC_EPS_BB2_IOUT_CH\n");
        read_current(ADC_EPS_BB2_IOUT_CH);
        // print("ADC_EPS_BT1_IOUT_CH\n");
        // read_current(ADC_EPS_BT1_IOUT_CH);
        print("ADC_EPS_BATT_IOUT_CH\n");
        read_current(ADC_EPS_BATT_IOUT_CH);

        // print("ADC_EPS_PV_POS_X_IOUT_CH\n");
        // read_current(ADC_EPS_PV_POS_X_IOUT_CH);
        // print("ADC_EPS_PV_NEG_X_IOUT_CH\n");
        // read_current(ADC_EPS_PV_NEG_X_IOUT_CH);
        // print("ADC_EPS_PV_POS_Y_IOUT_CH\n");
        // read_current(ADC_EPS_PV_POS_Y_IOUT_CH);
        // print("ADC_EPS_PV_NEG_Y_IOUT_CH\n");
        // read_current(ADC_EPS_PV_NEG_Y_IOUT_CH);

        print("\n");
        _delay_ms(5000);
    }
}

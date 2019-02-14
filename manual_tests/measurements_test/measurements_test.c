#include <uart/uart.h>
#include <adc/adc.h>
#include <adc/eps.h>
#include <spi/spi.h>
#include <conversions/conversions.h>

pin_info_t adc_cs = {
    .port = &ADC_CS_PORT_EPS,
    .ddr = &ADC_CS_DDR_EPS,
    .pin = ADC_CS_PIN_EPS
};

adc_t adc = {
    .channels = 0x0000, // don't poll any ADC pins in auto-1 mode
    .cs = &adc_cs
};

void read_voltage(uint8_t channel) {
    fetch_channel(&adc, channel);
    uint16_t raw_data = read_channel(&adc, channel);
    double raw_voltage = adc_raw_data_to_raw_vol(raw_data);
    double voltage = adc_raw_vol_to_eps_vol(raw_voltage);
    print("Channel: %u, Raw Data: 0x%04x, Raw Voltage: %ld mV, Voltage: %ld mV\n\n",
            channel, raw_data, (int32_t) (raw_voltage * 1000.0), (int32_t) (voltage * 1000.0));
}

void read_current(uint8_t channel) {
    fetch_channel(&adc, channel);
    uint16_t raw_data = read_channel(&adc, channel);
    double raw_voltage = adc_raw_data_to_raw_vol(raw_data);
    double current = adc_raw_vol_to_eps_cur(raw_voltage);
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

    init_adc(&adc);
    print("ADC Initialized\n\n");

    print("Starting test\n\n");

    while(1) {
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

        print("\n");
        _delay_ms(5000);
    }
}

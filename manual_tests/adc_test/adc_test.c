#include <adc/adc.h>
#include <conversions/conversions.h>
#include <spi/spi.h>
#include <uart/uart.h>

#include "../../src/devices.h"


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


int main(void){
    init_uart();
    print("\n\nUART initialized\n");

    init_spi();
    print("SPI Initialized\n");

    init_adc(&adc);
    print("ADC Initialized\n");

    print("\nStarting test\n\n");

    while (1) {

        fetch_all_adc_channels(&adc);

        print("Name, Channel, Raw Data, Raw Voltage, Converted Data\n");

        read_therm("ADC THM BATT2",       ADC_THM_BATT2);
        read_therm("ADC THM BATT1",       ADC_THM_BATT1);
        read_therm("ADC THM PAY CONN",    ADC_THM_PAY_CONN);
        read_voltage("ADC VMON 5V",       ADC_VMON_5V);
        read_current("ADC IMON X PLUS",   ADC_IMON_X_PLUS);
        read_current("ADC IMON X MINUS",  ADC_IMON_X_MINUS );
        read_current("ADC IMON Y PLUS",   ADC_IMON_Y_PLUS);
        read_current("ADC IMON Y MINUS",  ADC_IMON_Y_MINUS);
        read_current("ADC IMON 5V",       ADC_IMON_5V);
        read_therm("ADC THM 5V TOP",      ADC_THM_5V_TOP);
        read_voltage("ADC VMON 3V3",      ADC_VMON_3V3);
        read_current("ADC IMON 3V3",      ADC_IMON_3V3);
        read_therm("ADC THM 3V3 TOP",     ADC_THM_3V3_TOP);
        read_voltage("ADC VMON PACK",     ADC_VMON_PACK);
        read_current("ADC IMON PACK",     ADC_IMON_PACK);
        read_current("ADC IMON PAY LIM",  ADC_IMON_PAY_LIM);

        print("\n");
        _delay_ms(2000);
    }

    return 0;
}

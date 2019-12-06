#include <adc/adc.h>
#include <conversions/conversions.h>
#include <spi/spi.h>
#include <uart/uart.h>

#include "../../src/devices.h"


void read_voltage(char* name, uint8_t channel) {
    fetch_adc_channel(&adc, channel);
    uint16_t raw_data = read_adc_channel(&adc, channel);
    double ch_voltage = adc_raw_to_ch_vol(raw_data);
    double voltage = adc_raw_to_circ_vol(raw_data, ADC_VOL_SENSE_LOW_RES, ADC_VOL_SENSE_HIGH_RES);
    // String is left-justified, minimum 40 width
    print("%-15s, %u, 0x%.3x, %.3f V, %.3f V\n",
            name, channel, raw_data, ch_voltage, voltage);
}

void read_current(char* name, uint8_t channel, double sense_res, double ref_vol) {
    fetch_adc_channel(&adc, channel);
    uint16_t raw_data = read_adc_channel(&adc, channel);
    double ch_voltage = adc_raw_to_ch_vol(raw_data);
    double current = adc_raw_to_circ_cur(raw_data, sense_res, ref_vol);
    print("%-15s, %u, 0x%.3x, %.3f V, %.3f A\n",
            name, channel, raw_data, ch_voltage, current);
}

void read_therm(char* name, uint8_t channel) {
    fetch_adc_channel(&adc, channel);
    uint16_t raw_data = read_adc_channel(&adc, channel);
    double ch_voltage = adc_raw_to_ch_vol(raw_data);
    double temp = adc_raw_to_therm_temp(raw_data);
    print("%-15s, %u, 0x%.3x, %.3f V, %.3f C\n",
            name, channel, raw_data, ch_voltage, temp);
}

// This test reads the raw data and voltages on each ADC channel
// It converts the raw voltages to actual voltages and currents
int main(void){
    init_uart();
    print("\n\nUART initialized\n");

    init_spi();
    print("SPI Initialized\n");

    init_adc(&adc);
    print("ADC Initialized\n");

    print("\nStarting test\n\n");

    while (1) {
        print("Name, Channel, Raw Data, Channel Voltage, Converted Data\n");

        read_therm("THM BATT2",       ADC_THM_BATT2);
        read_therm("THM BATT1",       ADC_THM_BATT1);
        read_therm("THM PAY CONN",    ADC_THM_PAY_CONN);
        read_voltage("VMON 5V",       ADC_VMON_5V);
        read_current("IMON X PLUS",   ADC_IMON_X_PLUS, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF);
        read_current("IMON X MINUS",  ADC_IMON_X_MINUS, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF);
        read_current("IMON Y PLUS",   ADC_IMON_Y_PLUS, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF);
        read_current("IMON Y MINUS",  ADC_IMON_Y_MINUS, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF);
        read_current("IMON 5V",       ADC_IMON_5V, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF);
        read_therm("THM 5V TOP",      ADC_THM_5V_TOP);
        read_voltage("VMON 3V3",      ADC_VMON_3V3);
        read_current("IMON 3V3",      ADC_IMON_3V3, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF);
        read_therm("THM 3V3 TOP",     ADC_THM_3V3_TOP);
        read_voltage("VMON PACK",     ADC_VMON_PACK);
        read_current("IMON PACK",     ADC_IMON_PACK, ADC_BAT_CUR_SENSE_RES, ADC_BAT_CUR_SENSE_VREF);
        read_current("IMON PAY LIM",  ADC_IMON_PAY_LIM, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF);

        print("\n");
        _delay_ms(5000);
    }

    return 0;
}

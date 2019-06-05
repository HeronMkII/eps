/*
Comparator functionality for heaters. Requires thermistors and heaters/ammeter
attached to EPS board. This is very similar to the version on PAY.

Author: Brytni Richards, Bruno Almeida

For IC COMPARATOR LO PWR SGL V SOT 23-5L Part # TS391ILT
Digikey Link: https://www.digikey.ca/product-detail/en/stmicroelectronics/TS391ILT/497-2275-1-ND/599229
Datasheet: https://www.st.com/content/ccc/resource/technical/document/datasheet/de/4c/b3/3d/64/7d/48/8e/CD00001660.pdf/files/CD00001660.pdf/jcr:content/translations/en.CD00001660.pdf
*/

#include <stdbool.h>

#include <adc/adc.h>
#include <uart/uart.h>

#include "devices.h"
#include "heaters.h"
#include "measurements.h"

#define SETPOINT_SHADOW_HEATER1             20   //Celcius
#define SETPOINT_SHADOW_HEATER2             20   //Celcius
#define SETPOINT_SUN_HEATER1                5    //Celcius
#define SETPOINT_SUN_HEATER2                5    //Celcius

double current_threshold_upper = 1;    //Amps
double current_threshold_lower = 0.95; //Amps

// init is covered by dac_init()

// Sets temperature setpoint of heater 1 (connected to DAC A)
// raw_data - 12 bit DAC raw data for setpoint
void set_heater_1_raw_setpoint(uint16_t raw_data) {
    set_dac_raw_voltage(&dac, DAC_A, raw_data);
    //save to EEPROM
    eeprom_write_dword(HEATER_1_RAW_SETPOINT_ADDR,raw_data);
}

// Sets temperature setpoint of heater 2 (connected to DAC B)
// raw_data - 12 bit DAC raw data for setpoint
void set_heater_2_raw_setpoint(uint16_t raw_data) {
    set_dac_raw_voltage(&dac, DAC_B, raw_data);
    //save to EEPROM
    eeprom_write_dword(HEATER_2_RAW_SETPOINT_ADDR,raw_data);
}

// temp - in C
void set_heater_1_temp_setpoint(double temp) {
    double res = therm_temp_to_res(temp);
    double vol = therm_res_to_vol(res);
    uint16_t raw_data = dac_vol_to_raw_data(vol);
    set_heater_1_raw_setpoint(raw_data);
}

// temp - in C
void set_heater_2_temp_setpoint(double temp) {
    double res = therm_temp_to_res(temp);
    double vol = therm_res_to_vol(res);
    uint16_t raw_data = dac_vol_to_raw_data(vol);
    set_heater_2_raw_setpoint(raw_data);
}

void init_heaters() {
    uint16_t heater_1_last_setpoint = eeprom_read_dword(HEATER_1_RAW_SETPOINT_ADDR);
    if (heater_1_last_setpoint == EEPROM_DEF_DWORD){
        heater_1_last_setpoint = 0;
    }
    uint16_t heater_2_last_setpoint = eeprom_read_dword(HEATER_2_RAW_SETPOINT_ADDR);
    if (heater_2_last_setpoint == EEPROM_DEF_DWORD){
        heater_2_last_setpoint = 0;
    }
    set_heater_1_raw_setpoint(heater_1_last_setpoint);
    set_heater_2_raw_setpoint(heater_2_last_setpoint);
}

double read_current(uint8_t channel) {
    fetch_adc_channel(&adc, channel);
    uint16_t raw_data = read_adc_channel(&adc, channel);
    double current = adc_raw_data_to_eps_cur(raw_data);
    print(", %.6f", current);
    return current;
}

void change_setpoint(int setpoint1, int setpoint2)
{
    set_heater_1_temp_setpoint(setpoint1);
    print(", %.6f", setpoint1);
    set_heater_2_temp_setpoint(setpoint2);
    print(", %.6f", setpoint2);
}

//when called, will check if setpoint needs to be changed and then do so if needed
void sunorshadow_setpoint()
{
    double total_current = 0;
    total_current += read_current(MEAS_NEG_Y_IOUT);
    total_current += read_current(MEAS_POS_X_IOUT);
    total_current += read_current(MEAS_POS_Y_IOUT);
    total_current += read_current(MEAS_NEG_X_IOUT);

    if (total_current > current_threshold_upper)
    { //In the sun
        change_setpoint(SETPOINT_SUN_HEATER1, SETPOINT_SUN_HEATER2);
    }
    else if (total_current < current_threshold_lower)
    {
        change_setpoint(SETPOINT_SHADOW_HEATER1, SETPOINT_SHADOW_HEATER2);
    }
}

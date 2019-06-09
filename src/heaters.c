/*
Comparator functionality for heaters. Requires thermistors and heaters/ammeter
attached to EPS board. This is very similar to the version on PAY.

Author: Brytni Richards, Bruno Almeida

For IC COMPARATOR LO PWR SGL V SOT 23-5L Part # TS391ILT
Digikey Link: https://www.digikey.ca/product-detail/en/stmicroelectronics/TS391ILT/497-2275-1-ND/599229
Datasheet: https://www.st.com/content/ccc/resource/technical/document/datasheet/de/4c/b3/3d/64/7d/48/8e/CD00001660.pdf/files/CD00001660.pdf/jcr:content/translations/en.CD00001660.pdf

Shadow is the default setpoint mode, sun is the secondary mode.
*/

#include <stdbool.h>

#include <adc/adc.h>
#include <uart/uart.h>

#include "devices.h"
#include "heaters.h"
#include "measurements.h"

// Setpoints for different shadow/sun conditions (in raw 12-bit format)
uint16_t heater_1_raw_shadow_setpoint = 0;
uint16_t heater_2_raw_shadow_setpoint = 0;
uint16_t heater_1_raw_sun_setpoint = 0;
uint16_t heater_2_raw_sun_setpoint = 0;

double heater_sun_cur_thresh_upper = 1;    //Amps
double heater_sun_cur_thresh_lower = 0.95; //Amps

heater_mode_t heater_mode = HEATER_MODE_SHADOW;


uint32_t read_eeprom(uint32_t* addr) {
    uint32_t data = eeprom_read_dword(addr);
    if (data == EEPROM_DEF_DWORD){
        return 0;
    }
    return data;
}

void init_heaters(void) {
    heater_1_raw_shadow_setpoint =
        (uint16_t) read_eeprom(HEATER_1_RAW_SHADOW_SETPOINT_ADDR);
    heater_2_raw_shadow_setpoint =
        (uint16_t) read_eeprom(HEATER_2_RAW_SHADOW_SETPOINT_ADDR);
    heater_1_raw_sun_setpoint =
        (uint16_t) read_eeprom(HEATER_1_RAW_SUN_SETPOINT_ADDR);
    heater_2_raw_sun_setpoint =
        (uint16_t) read_eeprom(HEATER_2_RAW_SUN_SETPOINT_ADDR);

    update_heater_setpoints();
}

// Sets temperature setpoint of heater 1 (connected to DAC A)
// raw_data - 12 bit DAC raw data for setpoint
void set_heater_1_raw_shadow_setpoint(uint16_t raw_data) {
    heater_1_raw_shadow_setpoint = raw_data;
    //save to EEPROM
    eeprom_write_dword(HEATER_1_RAW_SHADOW_SETPOINT_ADDR, raw_data);
    update_heater_setpoints();
}

// Sets temperature setpoint of heater 2 (connected to DAC B)
// raw_data - 12 bit DAC raw data for setpoint
void set_heater_2_raw_shadow_setpoint(uint16_t raw_data) {
    heater_2_raw_shadow_setpoint = raw_data;
    //save to EEPROM
    eeprom_write_dword(HEATER_2_RAW_SHADOW_SETPOINT_ADDR, raw_data);
    update_heater_setpoints();
}

// Sets temperature setpoint of heater 1 (connected to DAC A)
// raw_data - 12 bit DAC raw data for setpoint
void set_heater_1_raw_sun_setpoint(uint16_t raw_data) {
    heater_1_raw_sun_setpoint = raw_data;
    //save to EEPROM
    eeprom_write_dword(HEATER_1_RAW_SUN_SETPOINT_ADDR, raw_data);
    update_heater_setpoints();
}

// Sets temperature setpoint of heater 2 (connected to DAC B)
// raw_data - 12 bit DAC raw data for setpoint
void set_heater_2_raw_sun_setpoint(uint16_t raw_data) {
    heater_2_raw_sun_setpoint = raw_data;
    //save to EEPROM
    eeprom_write_dword(HEATER_2_RAW_SUN_SETPOINT_ADDR, raw_data);
    update_heater_setpoints();
}

// temp - in C
// Returns - raw (12 bits)
uint16_t heater_setpoint_temp_to_raw(double temp) {
    double res = therm_temp_to_res(temp);
    double vol = therm_res_to_vol(res);
    uint16_t raw_data = dac_vol_to_raw_data(vol);
    return raw_data;
}


double read_eps_cur(uint8_t channel) {
    fetch_adc_channel(&adc, channel);
    uint16_t raw_data = read_adc_channel(&adc, channel);
    double current = adc_raw_data_to_eps_cur(raw_data);
    return current;
}

void update_heater_setpoints(void) {
    // Use shadow as the default just in case
    if (heater_mode == HEATER_MODE_SUN) {
        set_dac_raw_voltage(&dac, DAC_A, heater_1_raw_sun_setpoint);
        set_dac_raw_voltage(&dac, DAC_B, heater_2_raw_sun_setpoint);
    } else {
        set_dac_raw_voltage(&dac, DAC_A, heater_1_raw_shadow_setpoint);
        set_dac_raw_voltage(&dac, DAC_B, heater_2_raw_shadow_setpoint);
    }
}


//when called, will check if setpoint needs to be changed and then do so if needed
void control_heater_mode(void) {
    double total_current = 0;
    total_current += read_eps_cur(MEAS_NEG_Y_IOUT);
    total_current += read_eps_cur(MEAS_POS_X_IOUT);
    total_current += read_eps_cur(MEAS_POS_Y_IOUT);
    total_current += read_eps_cur(MEAS_NEG_X_IOUT);

    if (total_current > heater_sun_cur_thresh_upper) { //In the sun
        heater_mode = HEATER_MODE_SUN;
    }
    else if (total_current < heater_sun_cur_thresh_lower) {
        heater_mode = HEATER_MODE_SHADOW;
    }

    update_heater_setpoints();
}

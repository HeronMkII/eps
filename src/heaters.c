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
#include <uptime/uptime.h>

#include "devices.h"
#include "heaters.h"

heater_val_t heater_1_shadow_setpoint = {
    .raw = 0,
    .eeprom_addr = HEATER_1_SHADOW_SETPOINT_ADDR
};
heater_val_t heater_2_shadow_setpoint = {
    .raw = 0,
    .eeprom_addr = HEATER_2_SHADOW_SETPOINT_ADDR
};
heater_val_t heater_1_sun_setpoint = {
    .raw = 0,
    .eeprom_addr = HEATER_1_SUN_SETPOINT_ADDR
};
heater_val_t heater_2_sun_setpoint = {
    .raw = 0,
    .eeprom_addr = HEATER_2_SUN_SETPOINT_ADDR
};

heater_val_t heater_sun_cur_thresh_upper = {
    .raw = 0,
    .eeprom_addr = HEATER_CUR_THRESH_UPPER_ADDR
};
heater_val_t heater_sun_cur_thresh_lower = {
    .raw = 0,
    .eeprom_addr = HEATER_CUR_THRESH_LOWER_ADDR
};

heater_mode_t heater_mode = HEATER_MODE_SHADOW;


void init_heaters(void) {
    // Read setpoints
    heater_1_shadow_setpoint.raw = (uint16_t) read_eeprom_or_default(
        heater_1_shadow_setpoint.eeprom_addr,
        heater_setpoint_to_dac_raw_data(HEATER_1_DEF_SHADOW_SETPOINT));
    heater_2_shadow_setpoint.raw = (uint16_t) read_eeprom_or_default(
        heater_2_shadow_setpoint.eeprom_addr,
        heater_setpoint_to_dac_raw_data(HEATER_2_DEF_SHADOW_SETPOINT));
    heater_1_sun_setpoint.raw = (uint16_t) read_eeprom_or_default(
        heater_1_sun_setpoint.eeprom_addr,
        heater_setpoint_to_dac_raw_data(HEATER_1_DEF_SUN_SETPOINT));
    heater_2_sun_setpoint.raw = (uint16_t) read_eeprom_or_default(
        heater_2_sun_setpoint.eeprom_addr,
        heater_setpoint_to_dac_raw_data(HEATER_2_DEF_SUN_SETPOINT));

    // Don't need to call set_raw_heater_setpoint() to save to EEPROM because
    // when it restarts, it will use the default values again

    // Read current thresholds
    heater_sun_cur_thresh_upper.raw = (uint16_t) read_eeprom_or_default(
        heater_sun_cur_thresh_upper.eeprom_addr,
        adc_circ_cur_to_raw(HEATER_SUN_CUR_THRESH_UPPER,
        ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF));
    heater_sun_cur_thresh_lower.raw = (uint16_t) read_eeprom_or_default(
        heater_sun_cur_thresh_lower.eeprom_addr,
        adc_circ_cur_to_raw(HEATER_SUN_CUR_THRESH_LOWER,
        ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF));

    update_heater_setpoint_outputs();
}

// Sets temperature setpoint of heater 1 (connected to DAC A)
// raw_data - 12 bit DAC raw data for setpoint
void set_raw_heater_setpoint(heater_val_t* setpoint, uint16_t raw_data) {
    setpoint->raw = raw_data;
    //save to EEPROM
    write_eeprom(setpoint->eeprom_addr, setpoint->raw);
    update_heater_setpoint_outputs();
}

// Sets temperature setpoint of heater 1 (connected to DAC A)
// raw_data - 12 bit DAC raw data for setpoint
void set_raw_heater_cur_thresh(heater_val_t* cur_thresh, uint16_t raw_data) {
    cur_thresh->raw = raw_data;
    //save to EEPROM
    write_eeprom(cur_thresh->eeprom_addr, cur_thresh->raw);
    update_heater_setpoint_outputs();
}

double read_solar_cur(uint8_t channel) {
    fetch_adc_channel(&adc, channel);
    uint16_t raw_data = read_adc_channel(&adc, channel);
    double current = adc_raw_to_circ_cur(raw_data, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF);
    return current;
}

void update_heater_setpoint_outputs(void) {
    if (heater_mode == HEATER_MODE_SUN) {
        set_dac_raw_voltage(&dac, DAC_A, heater_1_sun_setpoint.raw);
        set_dac_raw_voltage(&dac, DAC_B, heater_2_sun_setpoint.raw);
    }
    // Use shadow as the default just in case
    else {
        set_dac_raw_voltage(&dac, DAC_A, heater_1_shadow_setpoint.raw);
        set_dac_raw_voltage(&dac, DAC_B, heater_2_shadow_setpoint.raw);
    }
}


//when called, will check if setpoint needs to be changed and then do so if needed
void control_heater_mode(void) {
    double total_current = 0;
    total_current += read_solar_cur(ADC_IMON_X_PLUS);
    total_current += read_solar_cur(ADC_IMON_X_MINUS);
    total_current += read_solar_cur(ADC_IMON_Y_PLUS);
    total_current += read_solar_cur(ADC_IMON_Y_MINUS);

    double upper_thresh = adc_raw_to_circ_cur(heater_sun_cur_thresh_upper.raw,
        ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF);
    double lower_thresh = adc_raw_to_circ_cur(heater_sun_cur_thresh_lower.raw,
        ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF);
    
    // TODO - might need to enable print float flags in main makefile
    print("Solar current: %.6f A\n", total_current);
    print("Upper threshold: %.6f A\n", upper_thresh);
    print("Lower threshold: %.6f A\n", lower_thresh);

    if (total_current > upper_thresh) { //In the sun
        heater_mode = HEATER_MODE_SUN;
        print("In sun mode\n");
    }
    else if (total_current < lower_thresh) {
        heater_mode = HEATER_MODE_SHADOW;
        print("In shadow mode\n");
    }

    update_heater_setpoint_outputs();
}

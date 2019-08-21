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
#include "measurements.h"

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

heater_mode_t heater_mode =  HEATER_MODE_SHADOW;
uint8_t low_power_countdown = 0;

void low_power_timer_func(void) {
    if (low_power_countdown > 0) {
        low_power_countdown -= 1;
        // Finished waiting
        if (low_power_countdown == 0) {
            update_heater_setpoint_outputs();
        }
    }
}

void init_heaters(void) {
    // Read setpoints
    heater_1_shadow_setpoint.raw = (uint16_t) read_eeprom(
        heater_1_shadow_setpoint.eeprom_addr,
        heater_setpoint_to_dac_raw_data(HEATER_1_DEF_SHADOW_SETPOINT));
    heater_2_shadow_setpoint.raw = (uint16_t) read_eeprom(
        heater_2_shadow_setpoint.eeprom_addr,
        heater_setpoint_to_dac_raw_data(HEATER_2_DEF_SHADOW_SETPOINT));
    heater_1_sun_setpoint.raw = (uint16_t) read_eeprom(
        heater_1_sun_setpoint.eeprom_addr,
        heater_setpoint_to_dac_raw_data(HEATER_1_DEF_SUN_SETPOINT));
    heater_2_sun_setpoint.raw = (uint16_t) read_eeprom(
        heater_2_sun_setpoint.eeprom_addr,
        heater_setpoint_to_dac_raw_data(HEATER_2_DEF_SUN_SETPOINT));

    // Don't need to call set_raw_heater_setpoint() to save to EEPROM because
    // when it restarts, it will use the default values again

    // Read current thresholds
    heater_sun_cur_thresh_upper.raw = (uint16_t) read_eeprom(
        heater_sun_cur_thresh_upper.eeprom_addr,
        adc_eps_cur_to_raw_data(HEATER_SUN_CUR_THRESH_UPPER));
    heater_sun_cur_thresh_lower.raw = (uint16_t) read_eeprom(
        heater_sun_cur_thresh_lower.eeprom_addr,
        adc_eps_cur_to_raw_data(HEATER_SUN_CUR_THRESH_LOWER));

    update_heater_setpoint_outputs();
    add_uptime_callback(low_power_timer_func);
}

// TODO - move to lib-common/utilities
// TODO - new struct eeprom_val_t to abstract reading/writing with default values
uint32_t read_eeprom(uint32_t* addr, uint32_t default_val) {
    uint32_t data = eeprom_read_dword(addr);
    if (data == EEPROM_DEF_DWORD){
        return default_val;
    }
    return data;
}

// Sets temperature setpoint of heater 1 (connected to DAC A)
// raw_data - 12 bit DAC raw data for setpoint
void set_raw_heater_setpoint(heater_val_t* setpoint, uint16_t raw_data) {
    setpoint->raw = raw_data;
    //save to EEPROM
    eeprom_write_dword(setpoint->eeprom_addr, setpoint->raw);
    update_heater_setpoint_outputs();
}

// Sets temperature setpoint of heater 1 (connected to DAC A)
// raw_data - 12 bit DAC raw data for setpoint
void set_raw_heater_cur_thresh(heater_val_t* cur_thresh, uint16_t raw_data) {
    cur_thresh->raw = raw_data;
    //save to EEPROM
    eeprom_write_dword(cur_thresh->eeprom_addr, cur_thresh->raw);
    update_heater_setpoint_outputs();
}

double read_eps_cur(uint8_t channel) {
    fetch_adc_channel(&adc, channel);
    uint16_t raw_data = read_adc_channel(&adc, channel);
    double current = adc_raw_data_to_eps_cur(raw_data);
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


// When called, will set the heaters to low power mode for 30 seconds
void start_low_power_mode(void) {
    // Go to low power mode
    set_dac_raw_voltage(&dac, DAC_A, 0);
    set_dac_raw_voltage(&dac, DAC_B, 0);
    // Wait 30 seconds before turning heaters back on
    low_power_countdown = 30;
}

//when called, will check if setpoint needs to be changed and then do so if needed
void control_heater_mode(void) {
    double total_current = 0;
    total_current += read_eps_cur(MEAS_NEG_Y_IOUT);
    total_current += read_eps_cur(MEAS_POS_X_IOUT);
    total_current += read_eps_cur(MEAS_POS_Y_IOUT);
    total_current += read_eps_cur(MEAS_NEG_X_IOUT);

    if (total_current > adc_raw_data_to_eps_cur(heater_sun_cur_thresh_upper.raw)) { //In the sun
        heater_mode = HEATER_MODE_SUN;
    }
    else if (total_current < adc_raw_data_to_eps_cur(heater_sun_cur_thresh_lower.raw)) {
        heater_mode = HEATER_MODE_SHADOW;
    }

    update_heater_setpoint_outputs();
}

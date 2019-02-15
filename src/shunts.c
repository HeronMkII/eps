/*
Controls the shunts (MOSFET transistors) that control the connection between the
solar panels and batteries.
*/

#include "shunts.h"

// true - shunts are ON, battery charging is OFF
// false - shunts are OFF, battery charging is ON
bool are_shunts_on = false;

// Initializes shunts as output pins
void init_shunts(void) {
    init_pex(&pex);
    set_pex_pin_dir(&pex, PEX_A, SHUNTS_POS_X, OUTPUT);
    set_pex_pin_dir(&pex, PEX_A, SHUNTS_NEG_X, OUTPUT);
    set_pex_pin_dir(&pex, PEX_A, SHUNTS_POS_Y, OUTPUT);
    set_pex_pin_dir(&pex, PEX_A, SHUNTS_NEG_Y, OUTPUT);

    init_adc(&adc);

    turn_shunts_off();
}

// Turns all shunt MOSFETS ON, meaning the current from the solar panels will be
// shorted to ground and the batteries STOP CHARGING
void turn_shunts_on(void) {
    set_pex_pin(&pex, PEX_A, SHUNTS_POS_X, 1);
    set_pex_pin(&pex, PEX_A, SHUNTS_NEG_X, 1);
    set_pex_pin(&pex, PEX_A, SHUNTS_POS_Y, 1);
    set_pex_pin(&pex, PEX_A, SHUNTS_NEG_Y, 1);

    are_shunts_on = true;
}

// Turns all shunt MOSFETS OFF, meaning the current from the solar panels will
// go to the battery and the batteries START CHARGING
void turn_shunts_off(void) {
    set_pex_pin(&pex, PEX_A, SHUNTS_POS_X, 0);
    set_pex_pin(&pex, PEX_A, SHUNTS_NEG_X, 0);
    set_pex_pin(&pex, PEX_A, SHUNTS_POS_Y, 0);
    set_pex_pin(&pex, PEX_A, SHUNTS_NEG_Y, 0);

    are_shunts_on = false;
}

void control_shunts(void) {
    // Read battery voltage
    uint8_t channel = MEAS_PACK_VOUT;
    fetch_channel(&adc, channel);
    uint16_t raw_data_pos = read_channel(&adc, channel);
    double batt_voltage = adc_raw_data_to_eps_vol(raw_data_pos);
    print("Battery Voltage: %.6f V\n", batt_voltage);

    // Decide whether to switch the shunts on, off, or stay the same
    if (!are_shunts_on && batt_voltage > SHUNTS_ON_THRESHOLD) {
        turn_shunts_on();
        print("Shunts on\n");
    } else if (are_shunts_on && batt_voltage < SHUNTS_OFF_THRESHOLD) {
        turn_shunts_off();
        print("Shunts off\n");
    } else {
        // No change in shunts
    }
}

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
    set_pex_pin_dir(&pex, PEX_A, PEX_EPS_SHUNT_PX_PIN, OUTPUT);
    set_pex_pin_dir(&pex, PEX_A, PEX_EPS_SHUNT_NX_PIN, OUTPUT);
    set_pex_pin_dir(&pex, PEX_A, PEX_EPS_SHUNT_PY_PIN, OUTPUT);
    set_pex_pin_dir(&pex, PEX_A, PEX_EPS_SHUNT_NY_PIN, OUTPUT);

    init_adc(&adc);

    turn_shunts_off();
}

// Turns all shunt MOSFETS ON, meaning the current from the solar panels will be
// shorted to ground and the batteries STOP CHARGING
void turn_shunts_on(void) {
    set_pex_pin(&pex, PEX_A, PEX_EPS_SHUNT_PX_PIN, 1);
    set_pex_pin(&pex, PEX_A, PEX_EPS_SHUNT_NX_PIN, 1);
    set_pex_pin(&pex, PEX_A, PEX_EPS_SHUNT_PY_PIN, 1);
    set_pex_pin(&pex, PEX_A, PEX_EPS_SHUNT_NY_PIN, 1);

    are_shunts_on = true;
}

// Turns all shunt MOSFETS OFF, meaning the current from the solar panels will
// go to the battery and the batteries START CHARGING
void turn_shunts_off(void) {
    set_pex_pin(&pex, PEX_A, PEX_EPS_SHUNT_PX_PIN, 0);
    set_pex_pin(&pex, PEX_A, PEX_EPS_SHUNT_NX_PIN, 0);
    set_pex_pin(&pex, PEX_A, PEX_EPS_SHUNT_PY_PIN, 0);
    set_pex_pin(&pex, PEX_A, PEX_EPS_SHUNT_NY_PIN, 0);

    are_shunts_on = false;
}

void control_shunts(void) {
    uint8_t channel;

    // Read positive battery voltage
    channel = ADC_EPS_BATT_VPOS_CH;
    fetch_channel(&adc, channel);
    uint16_t raw_data_pos = read_channel(&adc, channel);
    double voltage_pos = adc_raw_data_to_eps_vol(raw_data_pos);
    print("Positive Voltage: %.6f V\n", voltage_pos);

    // Read negative battery voltage
    channel = ADC_EPS_BATT_VNEG_CH;
    fetch_channel(&adc, channel);
    uint16_t raw_data_neg = read_channel(&adc, channel);
    double voltage_neg = adc_raw_data_to_eps_vol(raw_data_neg);
    print("Negative Voltage: %.6f V\n", voltage_neg);

    // Calculate differential battery voltage
    // TODO - maybe take absolute value?
    double batt_voltage = voltage_pos - voltage_neg;
    print("Battery Voltage: %.6f V\n", batt_voltage);

    // Decide whether to switch the shunts on, off, or stay the same
    if (!are_shunts_on && batt_voltage > SHUNTS_ON_BATT_VOUT_THRESHOLD) {
        turn_shunts_on();
        print("Shunts on\n");
    } else if (are_shunts_on && batt_voltage < SHUNTS_OFF_BATT_VOUT_THRESHOLD) {
        turn_shunts_off();
        print("Shunts off\n");
    } else {
        // No change in shunts
    }
}

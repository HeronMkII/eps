/*
Controls the shunts (MOSFET transistors) that control the connection between the
solar panels and batteries.

TODO - account for weird voltage spikes in +PACK (battery voltage), don't want
       to keep switching the shunts because of single-measurement voltage spikes
    - e.g. measure voltage, delay 1/10 of period (of repeating voltage
      fluctuations), measure voltage, delay 1/10 of period, measure voltage,
      average voltages, use that to determine whether to switch the shunts
*/

#include "shunts.h"

// Uncomment for extra logging
// #define SHUNTS_DEBUG

// true - shunts are ON, battery charging is OFF
// false - shunts are OFF, battery charging is ON
bool are_shunts_on = false;

// If the battery output voltage goes above this threshold (in V),
// the shunts should be turned on (battery charging off)
double shunts_on_threshold = SHUNTS_ON_DEF_THRESHOLD;
// If the battery output voltage goes below this threshold (in V),
// the shunts should be turned off (battery charging on)
double shunts_off_threshold = SHUNTS_OFF_DEF_THRESHOLD;

// Initializes shunts as output pins
void init_shunts(void) {
    set_pex_pin_dir(&pex, PEX_A, SHUNTS_POS_X, OUTPUT);
    set_pex_pin_dir(&pex, PEX_A, SHUNTS_NEG_X, OUTPUT);
    set_pex_pin_dir(&pex, PEX_A, SHUNTS_POS_Y, OUTPUT);
    set_pex_pin_dir(&pex, PEX_A, SHUNTS_NEG_Y, OUTPUT);

    // Turn shunts off (battery charging on) by default
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

    print("Shunts ON (charging OFF)\n");
}

// Turns all shunt MOSFETS OFF, meaning the current from the solar panels will
// go to the battery and the batteries START CHARGING
void turn_shunts_off(void) {
    set_pex_pin(&pex, PEX_A, SHUNTS_POS_X, 0);
    set_pex_pin(&pex, PEX_A, SHUNTS_NEG_X, 0);
    set_pex_pin(&pex, PEX_A, SHUNTS_POS_Y, 0);
    set_pex_pin(&pex, PEX_A, SHUNTS_NEG_Y, 0);

    are_shunts_on = false;

    print("Shunts OFF (charging ON)\n");
}

void control_shunts(void) {
    // Read battery voltage
    uint8_t channel = ADC_VMON_PACK;
    fetch_adc_channel(&adc, channel);
    uint16_t raw_data = read_adc_channel(&adc, channel);
    double batt_voltage = adc_raw_to_circ_vol(raw_data,
        ADC_VOL_SENSE_LOW_RES, ADC_VOL_SENSE_HIGH_RES);

#ifdef SHUNTS_DEBUG
    print("Battery Voltage: %.3f V\n", batt_voltage);
#endif

    // Decide whether to switch the shunts on, off, or stay the same
    if ((!are_shunts_on) && (batt_voltage > shunts_on_threshold)) {
        turn_shunts_on();
    } else if (are_shunts_on && (batt_voltage < shunts_off_threshold)) {
        turn_shunts_off();
    }
}

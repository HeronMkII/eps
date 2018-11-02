/*
Controls the shunts (MOSFET transistors) that control the connection between the
solar panels and batteries.
*/

#include "shunts.h"

// true - shunts are ON, battery charging is OFF
// false - shunts are OFF, battery charging is ON
bool are_shunts_on = true;

// Initializes shunts as output pins
void init_shunts(void) {
    init_pex(&pex);
    pex_set_pin_dir(&pex, PEX_EPS_SHUNT_PX_PIN, PEX_A, OUTPUT);
    pex_set_pin_dir(&pex, PEX_EPS_SHUNT_NX_PIN, PEX_A, OUTPUT);
    pex_set_pin_dir(&pex, PEX_EPS_SHUNT_PY_PIN, PEX_A, OUTPUT);
    pex_set_pin_dir(&pex, PEX_EPS_SHUNT_NY_PIN, PEX_A, OUTPUT);

    init_adc(&adc);

    turn_shunts_on();
}

// Turns all shunt MOSFETS ON, meaning the current from the solar panels will be
// shorted to ground and the batteries STOP CHARGING
void turn_shunts_on(void) {
    pex_set_pin(&pex, PEX_EPS_SHUNT_PX_PIN, PEX_A, HIGH);
    pex_set_pin(&pex, PEX_EPS_SHUNT_NX_PIN, PEX_A, HIGH);
    pex_set_pin(&pex, PEX_EPS_SHUNT_PY_PIN, PEX_A, HIGH);
    pex_set_pin(&pex, PEX_EPS_SHUNT_NY_PIN, PEX_A, HIGH);

    are_shunts_on = true;
}

// Turns all shunt MOSFETS OFF, meaning the current from the solar panels will
// go to the battery and the batteries START CHARGING
void turn_shunts_off(void) {
    pex_set_pin(&pex, PEX_EPS_SHUNT_PX_PIN, PEX_A, LOW);
    pex_set_pin(&pex, PEX_EPS_SHUNT_NX_PIN, PEX_A, LOW);
    pex_set_pin(&pex, PEX_EPS_SHUNT_PY_PIN, PEX_A, LOW);
    pex_set_pin(&pex, PEX_EPS_SHUNT_NY_PIN, PEX_A, LOW);

    are_shunts_on = false;
}

void control_shunts(void) {
    uint8_t channel;

    // Read positive battery voltage
    channel = ADC_EPS_BATT_VPOS_CH;
    fetch_channel(&adc, channel);
    uint16_t raw_data_pos = read_channel(&adc, channel);
    double raw_voltage_pos = adc_raw_data_to_raw_voltage(raw_data_pos);
    double voltage_pos = adc_eps_raw_voltage_to_voltage(raw_voltage_pos);
    print("Positive Voltage\n");
    print("Channel: %u, Raw Data: 0x%04x, Raw Voltage: %d V, Voltage: %d V\n\n",
            channel, raw_data_pos, (int8_t) raw_voltage_pos, (int8_t) voltage_pos);

    // Read negative battery voltage
    channel = ADC_EPS_BATT_VNEG_CH;
    fetch_channel(&adc, channel);
    uint16_t raw_data_neg = read_channel(&adc, channel);
    double raw_voltage_neg = adc_raw_data_to_raw_voltage(raw_data_neg);
    double voltage_neg = adc_eps_raw_voltage_to_voltage(raw_voltage_neg);
    print("Negative Voltage\n");
    print("Channel: %u, Raw Data: 0x%04x, Raw Voltage: %d V, Voltage: %d V\n\n",
            channel, raw_data_neg, (int8_t) raw_voltage_neg, (int8_t) voltage_neg);

    // Calculate differential battery voltage
    // TODO - maybe take absolute value?
    double batt_voltage = voltage_pos - voltage_neg;
    print("Battery Voltage: %d V\n", (int8_t) batt_voltage);

    // Decide whether to switch the shunts on, off, or stay the same
    if (!are_shunts_on && batt_voltage > SHUNTS_ON_BATT_VOUT_THRESHOLD) {
        turn_shunts_on();
        print("Shunts on\n");
    } else if (are_shunts_on && batt_voltage < SHUNTS_OFF_BATT_VOUT_THRESHOLD) {
        turn_shunts_off();
        print("Shunts off\n");
    } else {
        // print("No change in shunts\n");
    }
}

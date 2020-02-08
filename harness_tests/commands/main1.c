#include <conversions/conversions.h>
#include <test/test.h>

#include "../../src/devices.h"
#include "../../src/heaters.h"

void default_values_test(void) {
    ASSERT_FP_EQ(dac_raw_data_to_heater_setpoint(HEATER_1_DEF_SHADOW_SETPOINT), 25.0);
    ASSERT_FP_EQ(dac_raw_data_to_heater_setpoint(HEATER_2_DEF_SHADOW_SETPOINT), 25.0);
    ASSERT_FP_EQ(dac_raw_data_to_heater_setpoint(HEATER_1_DEF_SUN_SETPOINT), 19.977);
    ASSERT_FP_EQ(dac_raw_data_to_heater_setpoint(HEATER_2_DEF_SUN_SETPOINT), 19.977);

    ASSERT_FP_EQ(adc_raw_to_circ_cur(
        HEATER_SUN_CUR_THRESH_UPPER, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF),
        0.299);
    ASSERT_FP_EQ(adc_raw_to_circ_cur(
        HEATER_SUN_CUR_THRESH_LOWER, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF),
        0.099);

    // To test possible raw values
    
    // print("0x%x\n", heater_setpoint_to_dac_raw_data(20));
    // print("0x%x\n", heater_setpoint_to_dac_raw_data(25));

    // print("0x%x\n", adc_circ_cur_to_raw(
    //     1.000, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF));
    // print("0x%x\n", adc_circ_cur_to_raw(
    //     0.950, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF));
    // print("0x%x\n", adc_circ_cur_to_raw(
    //     0.300, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF));
    // print("0x%x\n", adc_circ_cur_to_raw(
    //     0.100, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF));
}

test_t t1 = {.name = "default_values_test", .fn = default_values_test};

test_t* suite[] = { &t1 };

int main() {
    run_tests(suite, sizeof(suite) / sizeof(suite[0]));
    return 0;
}


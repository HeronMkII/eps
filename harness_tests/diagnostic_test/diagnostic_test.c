/**
 * Diagnostic test to verify hardware functionality for EPS. This consists of: 
 *      - taking measurements with devices and making sure they are within 
 *          expected range
 *      - turning on a heater shuld increase current from boost and battery
 *      - imu gyro vals should be small numbers
 */
#include <stdbool.h>

#include <adc/adc.h>
#include <conversions/conversions.h>
#include <uart/uart.h>
#include <test/test.h>

#include "../../src/devices.h"
#include "../../src/heaters.h"
#include "../../src/imu.h"
#include "../../src/measurements.h"

double read_voltage(char* name, uint8_t channel) {
    fetch_adc_channel(&adc, channel);
    uint16_t raw_data = read_adc_channel(&adc, channel);
    double voltage = adc_raw_data_to_eps_vol(raw_data);
    return voltage;
}

double read_current(char* name, uint8_t channel) {
    fetch_adc_channel(&adc, channel);
    uint16_t raw_data = read_adc_channel(&adc, channel);
    double current = adc_raw_data_to_eps_cur(raw_data);
    return current;
}

double read_bat_current(char* name, uint8_t channel) {
    fetch_adc_channel(&adc, channel);
    uint16_t raw_data = read_adc_channel(&adc, channel);
    double current = adc_raw_data_to_bat_cur(raw_data);
    return current;
}

double read_therm(char* name, uint8_t channel) {
    fetch_adc_channel(&adc, channel);
    uint16_t raw_data = read_adc_channel(&adc, channel);
    double temp = adc_raw_data_to_therm_temp(raw_data);
    return temp;
}

void read_voltage_test(void) {
    double bb_vout = read_voltage("BB VOUT", MEAS_BB_VOUT);
    double pack_vout = read_voltage("PACK VOUT", MEAS_PACK_VOUT);
    double bt_out = read_voltage("BT VOUT", MEAS_BT_VOUT);

    ASSERT_FP_GREATER(bb_vout, 0);
    ASSERT_FP_LESS(bb_vout, 0);
    ASSERT_FP_GREATER(pack_vout, 0);
    ASSERT_FP_LESS(pack_vout, 0);
    ASSERT_FP_GREATER(bt_out, 0);
    ASSERT_FP_LESS(bt_out, 0);
}

void read_current_test(void) {
    double bb_iout = read_current("BB IOUT", MEAS_BB_IOUT);
    double negY_iout = read_current("-Y IOUT", MEAS_NEG_Y_IOUT);
    double posX_iout = read_current("+X IOUT", MEAS_POS_X_IOUT);
    double posY_iout = read_current("+Y IOUT", MEAS_POS_Y_IOUT);
    double negX_iout = read_current("-X IOUT", MEAS_NEG_X_IOUT);
    double pack_iout = read_bat_current("PACK IOUT", MEAS_PACK_IOUT);
    double bt_iout = read_current("BT IOUT", MEAS_BT_IOUT);

    ASSERT_FP_GREATER(bb_iout, 0);
    ASSERT_FP_LESS(bb_iout, 0);
    ASSERT_FP_GREATER(negY_iout, 0);
    ASSERT_FP_LESS(negY_iout, 0);
    ASSERT_FP_GREATER(posX_iout, 0);
    ASSERT_FP_LESS(posX_iout, 0);
    ASSERT_FP_GREATER(posY_iout, 0);
    ASSERT_FP_LESS(posY_iout, 0);
    ASSERT_FP_GREATER(negX_iout, 0);
    ASSERT_FP_LESS(negX_iout, 0);
    ASSERT_FP_GREATER(pack_iout, 0);
    ASSERT_FP_LESS(pack_iout, 0);
    ASSERT_FP_GREATER(bt_iout, 0);
    ASSERT_FP_LESS(bt_iout, 0);
}

void read_temp_test(void) {
    double therm1 = read_therm("THERM 1", MEAS_THERM_1);
    double therm2 = read_therm("THERM 2", MEAS_THERM_2);
    ASSERT_FP_GREATER(therm1, 0);
    ASSERT_FP_LESS(therm1, 0);
    ASSERT_FP_GREATER(therm2, 0);
    ASSERT_FP_LESS(therm1, 0);
}

void heater_test(void) {
    // Turning the heaters on / off and checking their respective current values
    // Initial values
    double bt_init = read_current("BT IOUT", MEAS_BT_IOUT);
    double battery_init = read_bat_current("PACK IOUT", MEAS_PACK_IOUT);

    // Turn heaters on
    _delay_ms(1000);
    set_raw_heater_setpoint(&heater_1_shadow_setpoint, 0xFFF);
    set_raw_heater_setpoint(&heater_1_sun_setpoint, 0xFFF);
    set_raw_heater_setpoint(&heater_2_shadow_setpoint, 0xFFF);
    set_raw_heater_setpoint(&heater_2_sun_setpoint, 0xFFF);

    // Check current
    _delay_ms(1000);
    double bt_on = read_current("BT IOUT", MEAS_BT_IOUT);
    double battery_on = read_bat_current("PACK IOUT", MEAS_PACK_IOUT);
    ASSERT_FP_GREATER(bt_on, bt_init);
    ASSERT_FP_GREATER(battery_on, battery_init);


    // Turn heaters off
    set_raw_heater_setpoint(&heater_1_shadow_setpoint, 0);
    set_raw_heater_setpoint(&heater_1_sun_setpoint, 0);
    set_raw_heater_setpoint(&heater_2_shadow_setpoint, 0);
    set_raw_heater_setpoint(&heater_2_sun_setpoint, 0);

    _delay_ms(1000);
    
    // Check current
    double bt_off = read_current("BT IOUT", MEAS_BT_IOUT);
    double battery_off = read_bat_current("PACK IOUT", MEAS_PACK_IOUT);
    ASSERT_FP_EQ(bt_off, bt_init);
    ASSERT_FP_EQ(battery_off, battery_init);
}

void imu_test(void) {
    for (int i = 0; i < 5; i++) {
        uint16_t cal_x = 0, cal_y = 0, cal_z = 0;
        get_imu_cal_gyro(&cal_x, &cal_y, &cal_z);

        ASSERT_FP_GREATER(cal_x, 0);
        ASSERT_FP_LESS(cal_x, 0);
        ASSERT_FP_GREATER(cal_y, 0);
        ASSERT_FP_LESS(cal_y, 0);
        ASSERT_FP_GREATER(cal_z, 0);
        ASSERT_FP_LESS(cal_z, 0);

        uint16_t uncal_x = 0, uncal_y = 0, uncal_z = 0;
        get_imu_uncal_gyro(&uncal_x, &uncal_y, &uncal_z, NULL, NULL, NULL);
        ASSERT_FP_GREATER(uncal_x, 0);
        ASSERT_FP_LESS(uncal_x, 0);
        ASSERT_FP_GREATER(uncal_y, 0);
        ASSERT_FP_LESS(uncal_y, 0);
        ASSERT_FP_GREATER(uncal_z, 0);
        ASSERT_FP_LESS(uncal_z, 0);

        _delay_ms(250);
    }
}


test_t t1 = {.name = "read voltage", .fn = read_voltage_test};
test_t t2 = {.name = "read current", .fn = read_current_test};
test_t t3 = {.name = "read temp", .fn = read_temp_test};
test_t t4 = {.name = "test heater", .fn = heater_test};
test_t t5 = {.name = "test imu", .fn = imu_test};



test_t* suite[] = {&t1, &t2, &t3, &t4, &t5};

int main() {
    init_cs(PD0, &DDRD);
    set_cs_high(PD0, &PORTD);

    init_spi();
    init_dac(&dac);
    init_adc(&adc);

    init_heaters();
    init_imu();

    run_tests(suite, sizeof(suite) / sizeof(suite[0]));
    return 0;
}


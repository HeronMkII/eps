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
#include "../../src/shunts.h"


double read_voltage(uint8_t channel) {
    fetch_adc_channel(&adc, channel);
    uint16_t raw_data = read_adc_channel(&adc, channel);
    double voltage = adc_raw_data_to_eps_vol(raw_data);
    return voltage;
}

double read_current(uint8_t channel) {
    fetch_adc_channel(&adc, channel);
    uint16_t raw_data = read_adc_channel(&adc, channel);
    double current = adc_raw_data_to_eps_cur(raw_data);
    return current;
}

double read_bat_current(uint8_t channel) {
    fetch_adc_channel(&adc, channel);
    uint16_t raw_data = read_adc_channel(&adc, channel);
    double current = adc_raw_data_to_bat_cur(raw_data);
    return current;
}

double read_therm(uint8_t channel) {
    fetch_adc_channel(&adc, channel);
    uint16_t raw_data = read_adc_channel(&adc, channel);
    double temp = adc_raw_data_to_therm_temp(raw_data);
    return temp;
}

void read_voltage_test(void) {
    double bb_vout = read_voltage(MEAS_BB_VOUT);
    double pack_vout = read_voltage(MEAS_PACK_VOUT);
    double bt_out = read_voltage(MEAS_BT_VOUT);

    ASSERT_FP_GREATER(bb_vout, 3.2);
    ASSERT_FP_LESS(bb_vout, 3.4);
    ASSERT_FP_GREATER(pack_vout, 3.0);
    ASSERT_FP_LESS(pack_vout, 4.2);
    ASSERT_FP_GREATER(bt_out, 4.9);
    ASSERT_FP_LESS(bt_out, 5.1);
}

void read_current_test(void) {
    double bb_iout = read_current(MEAS_BB_IOUT);
    double negY_iout = read_current(MEAS_NEG_Y_IOUT);
    double posX_iout = read_current(MEAS_POS_X_IOUT);
    double posY_iout = read_current(MEAS_POS_Y_IOUT);
    double negX_iout = read_current(MEAS_NEG_X_IOUT);
    double pack_iout = read_bat_current(MEAS_PACK_IOUT);
    double bt_iout = read_current(MEAS_BT_IOUT);

    ASSERT_FP_GREATER(bb_iout, 0.02);
    ASSERT_FP_LESS(bb_iout, 0.1);
    ASSERT_FP_GREATER(negY_iout, 0.1);
    ASSERT_FP_LESS(negY_iout, 0.5);
    ASSERT_FP_GREATER(posX_iout, 0.1);
    ASSERT_FP_LESS(posX_iout, 0.5);
    ASSERT_FP_GREATER(posY_iout, 0.1);
    ASSERT_FP_LESS(posY_iout, 0.5);
    ASSERT_FP_GREATER(negX_iout, 0.1);
    ASSERT_FP_LESS(negX_iout, 0.5);
    ASSERT_FP_GREATER(pack_iout, -0.5);
    ASSERT_FP_LESS(pack_iout, 0.5);
    ASSERT_FP_GREATER(bt_iout, 0.02);
    ASSERT_FP_LESS(bt_iout, 1.0);
}

void read_temp_test(void) {
    double therm1 = read_therm(MEAS_THERM_1);
    double therm2 = read_therm(MEAS_THERM_2);
    ASSERT_FP_GREATER(therm1, 22.0);
    ASSERT_FP_LESS(therm1, 30.0);
    ASSERT_FP_GREATER(therm2, 22.0);
    ASSERT_FP_LESS(therm1, 30.0);
}

void heater_test(void) {
    // Turning the heaters on / off and checking their respective current values
    // Initial values
    double bt_init = read_current(MEAS_BT_IOUT);
    double battery_init = read_bat_current(MEAS_PACK_IOUT);

    // Turn heaters on
    _delay_ms(1000);
    set_raw_heater_setpoint(&heater_1_shadow_setpoint, 0xFFF);
    set_raw_heater_setpoint(&heater_1_sun_setpoint, 0xFFF);
    set_raw_heater_setpoint(&heater_2_shadow_setpoint, 0xFFF);
    set_raw_heater_setpoint(&heater_2_sun_setpoint, 0xFFF);

    // Check current
    _delay_ms(1000);
    double bt_on = read_current(MEAS_BT_IOUT);
    double battery_on = read_bat_current(MEAS_PACK_IOUT);
    ASSERT_FP_GREATER(bt_on, bt_init);
    ASSERT_FP_LESS(battery_on, battery_init);


    // Turn heaters off
    set_raw_heater_setpoint(&heater_1_shadow_setpoint, 0);
    set_raw_heater_setpoint(&heater_1_sun_setpoint, 0);
    set_raw_heater_setpoint(&heater_2_shadow_setpoint, 0);
    set_raw_heater_setpoint(&heater_2_sun_setpoint, 0);

    _delay_ms(1000);
    
    // Check current
    double bt_off = read_current(MEAS_BT_IOUT);
    double battery_off = read_bat_current(MEAS_PACK_IOUT);
    // Should be approximately equal to when the heaters were off
    ASSERT_FP_LESS(bt_off, bt_on);
    ASSERT_FP_GREATER(battery_off, battery_on);
}

void imu_test(void) {
    // TODO: take measurements multiple times, make sure at least one of them is not 0
    uint16_t raw_cal_x = 0, raw_cal_y = 0, raw_cal_z = 0;
    ASSERT_TRUE(get_imu_cal_gyro(&raw_cal_x, &raw_cal_y, &raw_cal_z));
    double cal_x = imu_raw_data_to_gyro(raw_cal_x);
    double cal_y = imu_raw_data_to_gyro(raw_cal_y);
    double cal_z = imu_raw_data_to_gyro(raw_cal_z);

    ASSERT_FP_GREATER(cal_x, -0.1);
    ASSERT_FP_LESS(cal_x, 0.1);
    ASSERT_FP_NEQ(cal_x, 0);
    ASSERT_FP_GREATER(cal_y, -0.1);
    ASSERT_FP_LESS(cal_y, 0.1);
    ASSERT_FP_NEQ(cal_y, 0);
    ASSERT_FP_GREATER(cal_z, -0.1);
    ASSERT_FP_LESS(cal_z, 0.1);
    ASSERT_FP_NEQ(cal_z, 0);


    uint16_t raw_uncal_x = 0, raw_uncal_y = 0, raw_uncal_z = 0;
    ASSERT_TRUE(get_imu_uncal_gyro(&raw_uncal_x, &raw_uncal_y, &raw_uncal_z, NULL, NULL, NULL));
    double uncal_x = imu_raw_data_to_gyro(raw_uncal_x);
    double uncal_y = imu_raw_data_to_gyro(raw_uncal_y);
    double uncal_z = imu_raw_data_to_gyro(raw_uncal_z);
    ASSERT_FP_GREATER(uncal_x, -0.1);
    ASSERT_FP_LESS(uncal_x, 0.1);
    ASSERT_FP_NEQ(uncal_x, 0);

    ASSERT_FP_GREATER(uncal_y, -0.1);
    ASSERT_FP_LESS(uncal_y, 0.1);
    ASSERT_FP_NEQ(uncal_y, 0);
    
    ASSERT_FP_GREATER(uncal_z, -0.1);
    ASSERT_FP_LESS(uncal_z, 0.1);
    ASSERT_FP_NEQ(uncal_z, 0);

}

test_t t1 = {.name = "read voltage", .fn = read_voltage_test};
test_t t2 = {.name = "read current", .fn = read_current_test};
test_t t3 = {.name = "read temp", .fn = read_temp_test};
test_t t4 = {.name = "test heater", .fn = heater_test};
test_t t5 = {.name = "test imu", .fn = imu_test};

test_t* suite[] = {&t1, &t2, &t3, &t4, &t5};

int main() {
    // UART
    init_uart();

    // SPI
    init_spi();

    // ADC
    init_adc(&adc);

    // PEX
    init_pex(&pex);

    // DAC
    init_dac(&dac);

    // Shunts
    init_shunts();

    // IMU
    init_imu();

    run_tests(suite, sizeof(suite) / sizeof(suite[0]));
    return 0;
}


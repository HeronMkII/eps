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

uint8_t construct_rx_msg_hk(uint8_t field_num){
    uint8_t rx_msg[8] = {0x00};
    rx_msg[2] = CAN_EPS_HK;
    rx_msg[3] = field_num;
    enqueue(&can_rx_msg_queue, rx_msg);
    handle_rx_msg();
    dequeue(&can_tx_msg_queue, tx_msg);
}

void read_voltage_test(void) {
    /* Battery voltage */
    /* Enqueue rx msg and handle */
    construct_rx_msg_hk(CAN_EPS_HK_BAT_VOL);

    /* Convert for assertion */
    uint16_t raw_data_bat = (tx_msg[4] << 8) & tx_msg[5];
    double bat_voltage = adc_raw_to_circ_vol(raw_data_bat, ADC_VOL_SENSE_LOW_RES, ADC_VOL_SENSE_HIGH_RES);

    /* Assert that voltage is within range */
    ASSERT_FP_GREATER(bat_voltage, 4.0);
    ASSERT_FP_LESS(bat_voltage, 4.2);

    /* 3V3 Voltage */
    construct_rx_msg_hk(CAN_EPS_HK_3V3_VOL);
    uint16_t raw_data_3v3 = (tx_msg[4] << 8) & tx_msg[5];
    double voltage_3v3 = adc_raw_to_circ_vol(raw_data_3v3, ADC_VOL_SENSE_LOW_RES, ADC_VOL_SENSE_HIGH_RES);

    /* Assert that voltage is within range */
    ASSERT_FP_GREATER(voltage_3v3, 3.29);
    ASSERT_FP_LESS(voltage_3v3, 3.31);

    /* 5V Voltage */
    construct_rx_msg_hk(CAN_EPS_HK_5V_VOL);
     (&can_tx_msg_queue, tx_msg);
    uint16_t raw_data_5v = (tx_msg[4] << 8) & tx_msg[5];
    double voltage_5v = adc_raw_to_circ_vol(raw_data_5v, ADC_VOL_SENSE_LOW_RES, ADC_VOL_SENSE_HIGH_RES);
    ASSERT_FP_GREATER(voltage_5v, 4.99);
    ASSERT_FP_LESS(voltage_5v, 5.01);
}

void read_current_test(void) {
    /* CAN_EPS_HK_BAT_CUR */
    construct_rx_msg_hk(CAN_EPS_HK_BAT_CUR);
    uint16_t raw_data = (tx_msg[4] << 8) & tx_msg[5];
    double current = adc_raw_to_circ_cur(raw_data, ADC_BAT_CUR_SENSE_RES, ADC_BAT_CUR_SENSE_VREF);
    /* Assert current is within range */
    ASSERT_FP_GREATER(current, 0.2);
    ASSERT_FP_LESS(current, 0.3);

    /* CAN_EPS_HK_X_POS_CUR */
    construct_rx_msg_hk(CAN_EPS_HK_X_POS_CUR);
    raw_data = (tx_msg[4] << 8) & tx_msg[5];
    current = adc_raw_to_circ_cur(raw_data, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF);
    /* Assert current is within range */
    ASSERT_FP_GREATER(current, 0.4);
    ASSERT_FP_LESS(current, 0.5);

    /* CAN_EPS_HK_X_NEG_CUR */
    construct_rx_msg_hk(CAN_EPS_HK_X_NEG_CUR);
    raw_data = (tx_msg[4] << 8) & tx_msg[5];
    current = adc_raw_to_circ_cur(raw_data, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF);
    /* Assert current is within range */
    ASSERT_FP_GREATER(current, 0.4);
    ASSERT_FP_LESS(current, 0.5);

    /* CAN_EPS_HK_Y_POS_CUR */
    construct_rx_msg_hk(CAN_EPS_HK_Y_POS_CUR);
    raw_data = (tx_msg[4] << 8) & tx_msg[5];
    current = adc_raw_to_circ_cur(raw_data, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF);
    /* Assert current is within range */
    ASSERT_FP_GREATER(current, 0.4);
    ASSERT_FP_LESS(current, 0.5);

    /* CAN_EPS_HK_Y_NEG_CUR */
    construct_rx_msg_hk(CAN_EPS_HK_Y_NEG_CUR);
    raw_data = (tx_msg[4] << 8) & tx_msg[5];
    current = adc_raw_to_circ_cur(raw_data, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF);
    /* Assert current is within range */
    ASSERT_FP_GREATER(current, 0.4);
    ASSERT_FP_LESS(current, 0.5);

    /* CAN_EPS_HK_3V3_CUR */
    construct_rx_msg_hk(CAN_EPS_HK_3V3_CUR);
    raw_data = (tx_msg[4] << 8) & tx_msg[5];
    current = adc_raw_to_circ_cur(raw_data, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF);
    /* Assert current is within range */
    ASSERT_FP_GREATER(current, 0.1);
    ASSERT_FP_LESS(current, 0.2);

    /* CAN_EPS_HK_5V_CUR */
    construct_rx_msg_hk(CAN_EPS_HK_5V_CUR);
    raw_data = (tx_msg[4] << 8) & tx_msg[5];
    current = adc_raw_to_circ_cur(raw_data, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF);
    /* Assert current is within range */
    ASSERT_FP_GREATER(current, 0.2);
    ASSERT_FP_LESS(current, 0.3);

    /* CAN_EPS_HK_PAY_CUR */
    construct_rx_msg_hk(CAN_EPS_HK_PAY_CUR);
    raw_data = (tx_msg[4] << 8) & tx_msg[5];
    current = adc_raw_to_circ_cur(raw_data, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF);
    /* Assert current is within range */
    ASSERT_FP_GREATER(current, 0.2);
    ASSERT_FP_LESS(current, 0.3);
}

void read_temp_test(void) {
    uint16_t raw_data_temp = 0;
    double temp = 0;
    /* Battery 1 Temperature */
    construct_rx_msg_hk(CAN_EPS_HK_BAT_TEMP1);
    raw_data_temp = (tx_msg[4] << 8) & tx_msg[5];
    temp = adc_raw_to_therm_temp(raw_data_temp);
    ASSERT_FP_GREATER(temp, 24.0);
    ASSERT_FP_LESS(temp, 26.0);

    /* Battery 2 Temperature */
    construct_rx_msg_hk(CAN_EPS_HK_BAT_TEMP2);
    raw_data_temp = (tx_msg[4] << 8) & tx_msg[5];
    temp = adc_raw_to_therm_temp(raw_data_temp);
    ASSERT_FP_GREATER(temp, 24.0);
    ASSERT_FP_LESS(temp, 26.0);

    /* 3v3 Buck-Boost Converter Temperature */
    construct_rx_msg_hk(CAN_EPS_HK_3V3_TEMP);
    raw_data_temp = (tx_msg[4] << 8) & tx_msg[5];
    temp = adc_raw_to_therm_temp(raw_data_temp);
    ASSERT_FP_GREATER(temp, 24.0);
    ASSERT_FP_LESS(temp, 26.0);

    /* 5V Boost Converter Temperature */
    construct_rx_msg_hk(CAN_EPS_HK_5V_TEMP);
    raw_data_temp = (tx_msg[4] << 8) & tx_msg[5];
    temp = adc_raw_to_therm_temp(raw_data_temp);
    ASSERT_FP_GREATER(temp, 24.0);
    ASSERT_FP_LESS(temp, 26.0);

    /* Payload Connector Temperature */
    construct_rx_msg_hk(CAN_EPS_HK_PAY_CON_TEMP);
    uint16_t raw_data_temp = (tx_msg[4] << 8) & tx_msg[5];
    double temp = adc_raw_to_therm_temp(raw_data_temp);
    ASSERT_FP_GREATER(temp, 24.0);
    ASSERT_FP_LESS(temp, 26.0);
}

void heater_test(void) {
    // Turning the heaters on / off and checking their respective current values

    // Initial values
    construct_rx_msg_hk(MEAS_BT_IOUT);
    uint16_t raw_data = (tx_msg[4] << 8) & tx_msg[5];
    double bt_init = adc_raw_to_circ_cur(raw_data, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF);
    construct_rx_msg_hk(MEAS_PACK_IOUT);
    uint16_t raw_data = (tx_msg[4] << 8) & tx_msg[5];
    double battery_init = adc_raw_to_circ_cur(raw_data, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF);

    // Turn heaters on
    _delay_ms(1000);
    set_raw_heater_setpoint(&heater_1_shadow_setpoint, 0xFFF);
    set_raw_heater_setpoint(&heater_1_sun_setpoint, 0xFFF);
    set_raw_heater_setpoint(&heater_2_shadow_setpoint, 0xFFF);
    set_raw_heater_setpoint(&heater_2_sun_setpoint, 0xFFF);

    // Check current
    _delay_ms(1000);
    construct_rx_msg_hk(MEAS_BT_IOUT);
    raw_data = (tx_msg[4] << 8) & tx_msg[5];
    double bt_on = adc_raw_to_circ_cur(raw_data, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF);
    construct_rx_msg_hk(MEAS_PACK_IOUT);
    raw_data = (tx_msg[4] << 8) & tx_msg[5];
    double battery_on = adc_raw_to_circ_cur(raw_data, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF);
    // Assert correct ranges
    ASSERT_FP_GREATER(bt_on, bt_init);
    ASSERT_FP_LESS(battery_on, battery_init);

    // Turn heaters off
    set_raw_heater_setpoint(&heater_1_shadow_setpoint, 0);
    set_raw_heater_setpoint(&heater_1_sun_setpoint, 0);
    set_raw_heater_setpoint(&heater_2_shadow_setpoint, 0);
    set_raw_heater_setpoint(&heater_2_sun_setpoint, 0);

    _delay_ms(1000);
    construct_rx_msg_hk(MEAS_BT_IOUT);
    raw_data = (tx_msg[4] << 8) & tx_msg[5];
    double bt_off = adc_raw_to_circ_cur(raw_data, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF);
    construct_rx_msg_hk(MEAS_PACK_IOUT);
    raw_data = (tx_msg[4] << 8) & tx_msg[5];
    double battery_off = adc_raw_to_circ_cur(raw_data, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF);
    // Should be approximately equal to when the heaters were off
    ASSERT_FP_LESS(bt_off, bt_on);
    ASSERT_FP_GREATER(battery_off, battery_on);
}

void imu_test(void) {
    uint16_t raw_data_imu = 0;
    double gyr_data = 0;
    uint8_t not_zero_flag = 0;

    for (int i=0; i<3; i++){
        construct_rx_msg_hk(CAN_EPS_HK_GYR_UNCAL_X);
        raw_data_imu = (tx_msg[4] << 8) & tx_msg[5];
        gyr_data = imu_raw_data_to_gyro(raw_data_imu);
        ASSERT_FP_GREATER(gyr_data, -0.1);
        ASSERT_FP_LESS(gyr_data, 0.1);
        /* Ensures that at least one measurement is non-zero */
        if (gyr_data != 0){
            not_zero_flag |= 1;
        }
    }
    ASSERT_TRUE(not_zero_flag);

    not_zero_flag = 0;
    for (int i=0; i<3; i++){
        construct_rx_msg_hk(CAN_EPS_HK_GYR_UNCAL_Y);
        raw_data_imu = (tx_msg[4] << 8) & tx_msg[5];
        gyr_data = imu_raw_data_to_gyro(raw_data_imu);
        ASSERT_FP_GREATER(gyr_data, -0.1);
        ASSERT_FP_LESS(gyr_data, 0.1);
        ASSERT_FP_NEQ(gyr_data, 0);
        /* Ensures that at least one measurement is non-zero */
        if (gyr_data != 0){
            not_zero_flag |= 1;
        }
    }
    ASSERT_TRUE(not_zero_flag);

    not_zero_flag = 0;
    for (int i=0; i<3; i++){
        construct_rx_msg_hk(CAN_EPS_HK_GYR_UNCAL_Z);
        raw_data_imu = (tx_msg[4] << 8) & tx_msg[5];
        gyr_data = imu_raw_data_to_gyro(raw_data_imu);
        ASSERT_FP_GREATER(gyr_data, -0.1);
        ASSERT_FP_LESS(gyr_data, 0.1);
        ASSERT_FP_NEQ(gyr_data, 0);
        /* Ensures that at least one measurement is non-zero */
        if (gyr_data != 0){
            not_zero_flag |= 1;
        }
    }
    ASSERT_TRUE(not_zero_flag);


    not_zero_flag = 0;
    for (int i=0; i<3; i++){
        construct_rx_msg_hk(CAN_EPS_HK_GYR_CAL_X);
        raw_data_imu = (tx_msg[4] << 8) & tx_msg[5];
        gyr_data = imu_raw_data_to_gyro(raw_data_imu);
        ASSERT_FP_GREATER(gyr_data, -0.1);
        ASSERT_FP_LESS(gyr_data, 0.1);
        ASSERT_FP_NEQ(gyr_data, 0);
        /* Ensures that at least one measurement is non-zero */
        if (gyr_data != 0){
            not_zero_flag |= 1;
        }
    }
    ASSERT_TRUE(not_zero_flag);

    for (int i=0; i<3; i++){
        construct_rx_msg_hk(CAN_EPS_HK_GYR_CAL_Y);
        raw_data_imu = (tx_msg[4] << 8) & tx_msg[5];
        gyr_data = imu_raw_data_to_gyro(raw_data_imu);
        ASSERT_FP_GREATER(gyr_data, -0.1);
        ASSERT_FP_LESS(gyr_data, 0.1);
        ASSERT_FP_NEQ(gyr_data, 0);
        /* Ensures that at least one measurement is non-zero */
        if (gyr_data != 0){
            not_zero_flag |= 1;
        }
    }
    ASSERT_TRUE(not_zero_flag);

    for (int i=0; i<3; i++){
        construct_rx_msg_hk(CAN_EPS_HK_GYR_CAL_Z);
        raw_data_imu = (tx_msg[4] << 8) & tx_msg[5];
        gyr_data = imu_raw_data_to_gyro(raw_data_imu);
        ASSERT_FP_GREATER(gyr_data, -0.1);
        ASSERT_FP_LESS(gyr_data, 0.1);
        ASSERT_FP_NEQ(gyr_data, 0);
        /* Ensures that at least one measurement is non-zero */
        if (gyr_data != 0){
            not_zero_flag |= 1;
        }
    }
    ASSERT_TRUE(not_zero_flag);
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

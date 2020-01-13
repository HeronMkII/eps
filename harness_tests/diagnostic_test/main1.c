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
#include <can/data_protocol.h>

#include "../../src/devices.h"
#include "../../src/heaters.h"
#include "../../src/imu.h"
#include "../../src/measurements.h"
#include "../../src/shunts.h"
#include "../../src/can_commands.h"

uint8_t tx_msg[8] = {0x00};

void construct_rx_msg_hk(uint8_t field_num){
    uint8_t rx_msg[8] = {0x00};
    rx_msg[2] = CAN_EPS_HK;
    rx_msg[3] = field_num;
    enqueue(&can_rx_msg_queue, rx_msg);
    handle_rx_msg();
    dequeue(&can_tx_msg_queue, tx_msg);
}

void construct_rx_msg_ctrl(uint8_t field_num, uint32_t data){
    uint8_t rx_msg[8] = {0x00};
    rx_msg[2] = CAN_EPS_CTRL;
    rx_msg[3] = field_num;
    tx_msg[4] = (data >> 24) & 0xFF;
    tx_msg[5] = (data >> 16) & 0xFF;
    tx_msg[6] = (data >> 8) & 0xFF;
    tx_msg[7] = data & 0xFF;
    enqueue(&can_rx_msg_queue, rx_msg);
    handle_rx_msg();
    dequeue(&can_tx_msg_queue, tx_msg);
}

double measure_heater_current(uint8_t source){
    construct_rx_msg_hk(source);
    uint16_t raw_data = (tx_msg[4] << 8) & tx_msg[5];
    double current = adc_raw_to_circ_cur(raw_data, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF);
    return current;
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
    raw_data_temp = (tx_msg[4] << 8) & tx_msg[5];
    temp = adc_raw_to_therm_temp(raw_data_temp);
    ASSERT_FP_GREATER(temp, 24.0);
    ASSERT_FP_LESS(temp, 26.0);
}

void heater_test(void) {
    // Verifies that battery pack current increases by between 0.15A and 0.2A for each heater
    // and between 0.3A and 0.4A for both heaters by first computing baseline current. Boost
    // converter current should increase by 0.12-0.16 and 0.24-0.32A as above.

    // Initial values
    double heater_off_curr_pack = measure_heater_current(MEAS_PACK_IOUT);
    double heater_off_curr_boost = measure_heater_current(MEAS_BT_IOUT);

    // Turn heater 1 on
    _delay_ms(1000);
    set_raw_heater_setpoint(&heater_1_shadow_setpoint, 0xFFF);
    set_raw_heater_setpoint(&heater_1_sun_setpoint, 0xFFF);

    // Check current due to heater 1 on
    _delay_ms(1000);
    double heater_on_curr_pack = measure_heater_current(MEAS_PACK_IOUT);
    double heater_on_curr_boost = measure_heater_current(MEAS_BT_IOUT);

    // Assert correct ranges for battery back and boost converter
    ASSERT_FP_GREATER(heater_on_curr_pack, heater_off_curr_pack + 0.15);
    ASSERT_FP_LESS(heater_on_curr_pack, heater_off_curr_pack + 0.2);

    ASSERT_FP_GREATER(heater_on_curr_boost, heater_off_curr_boost + 0.12);
    ASSERT_FP_LESS(heater_on_curr_boost, heater_off_curr_boost + 0.16);

    // Turn heater 1 off
    _delay_ms(1000);
    set_raw_heater_setpoint(&heater_1_shadow_setpoint, 0);
    set_raw_heater_setpoint(&heater_1_sun_setpoint, 0);

    // Turn heater 2 on
    set_raw_heater_setpoint(&heater_2_shadow_setpoint, 0xFFF);
    set_raw_heater_setpoint(&heater_2_sun_setpoint, 0xFFF);
    _delay_ms(1000);

    // Check current due to heater 2 on
    _delay_ms(1000);
    heater_on_curr_pack = measure_heater_current(MEAS_PACK_IOUT);
    heater_on_curr_boost = measure_heater_current(MEAS_BT_IOUT);

    // Assert correct ranges
    ASSERT_FP_GREATER(heater_on_curr_pack, heater_off_curr_pack + 0.15);
    ASSERT_FP_LESS(heater_on_curr_pack, heater_off_curr_pack + 0.2);

    ASSERT_FP_GREATER(heater_on_curr_boost, heater_off_curr_boost + 0.12);
    ASSERT_FP_LESS(heater_on_curr_boost, heater_off_curr_boost + 0.16);

    // Turn heater 1 on
    _delay_ms(1000);
    set_raw_heater_setpoint(&heater_1_shadow_setpoint, 0xFFF);
    set_raw_heater_setpoint(&heater_1_sun_setpoint, 0xFFF);

    // Check current due to both heaters on
    _delay_ms(1000);
    heater_on_curr_pack = measure_heater_current(MEAS_PACK_IOUT);
    heater_on_curr_boost = measure_heater_current(MEAS_BT_IOUT);

    // Assert correct ranges
    ASSERT_FP_GREATER(heater_on_curr_pack, heater_off_curr_pack + 0.3);
    ASSERT_FP_LESS(heater_on_curr_pack, heater_off_curr_pack + 0.4);

    ASSERT_FP_GREATER(heater_on_curr_boost, heater_off_curr_boost + 0.24);
    ASSERT_FP_LESS(heater_on_curr_boost, heater_off_curr_boost + 0.32);

    // Turn heaters 1 and 2 off
    set_raw_heater_setpoint(&heater_1_shadow_setpoint, 0);
    set_raw_heater_setpoint(&heater_1_sun_setpoint, 0);
    set_raw_heater_setpoint(&heater_2_shadow_setpoint, 0);
    set_raw_heater_setpoint(&heater_2_sun_setpoint, 0);
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

void heater_setpoint_test(void){
    construct_rx_msg_ctrl(CAN_EPS_CTRL_GET_HEAT_CUR_THRESH_LOWER, 10);
    construct_rx_msg_ctrl(CAN_EPS_CTRL_GET_HEAT_CUR_THRESH_UPPER, 10.05);
    control_heater_mode();
    ASSERT_EQ(heater_mode, HEATER_MODE_SHADOW);

    construct_rx_msg_ctrl(CAN_EPS_CTRL_GET_HEAT_CUR_THRESH_LOWER, 0);
    construct_rx_msg_ctrl(CAN_EPS_CTRL_GET_HEAT_CUR_THRESH_UPPER, 0.05);
    control_heater_mode();
    ASSERT_EQ(heater_mode, HEATER_MODE_SUN);
}

void uptime_test(void){
    construct_rx_msg_hk(CAN_EPS_HK_UPTIME);
    uint16_t tx_data = uptime_s;
    ASSERT_FP_GREATER(1000); // 1 second
    ASSERT_FP_LESS(10000); // 10 seconds
}

void restart_test(void){
    construct_rx_msg_hk(CAN_EPS_HK_RESTART_COUNT);
    uint16_t tx_data = restart_count;
    ASSERT_FP_GREATER(1);
    ASSERT_FP_LESS(1000);

    construct_rx_msg_hk(CAN_EPS_HK_RESTART_REASON);
    tx_data = restart_reason;
    ASSERT_EQ(0x06);
}

void temp_low_power_mode_test(void){
    // start temporary low-power mode for 30 s
    start_low_power_mode();

    /* CAN_EPS_HK_BAT_CUR */
    construct_rx_msg_hk(CAN_EPS_HK_BAT_CUR);
    uint16_t raw_data = (tx_msg[4] << 8) & tx_msg[5];
    double current = adc_raw_to_circ_cur(raw_data, ADC_BAT_CUR_SENSE_RES, ADC_BAT_CUR_SENSE_VREF);
    /* Assert current is within range */
    ASSERT_FP_GREATER(current, 0.1);
    ASSERT_FP_LESS(current, 0.2);
}

void indefinite_low_power_mode_test(void){
  // Go to low power mode
  set_dac_raw_voltage(&dac, DAC_A, 0);
  set_dac_raw_voltage(&dac, DAC_B, 0);

  /* CAN_EPS_HK_BAT_CUR */
  construct_rx_msg_hk(CAN_EPS_HK_BAT_CUR);
  uint16_t raw_data = (tx_msg[4] << 8) & tx_msg[5];
  double current = adc_raw_to_circ_cur(raw_data, ADC_BAT_CUR_SENSE_RES, ADC_BAT_CUR_SENSE_VREF);
  /* Assert current is within range */
  ASSERT_FP_GREATER(current, 0.1);
  ASSERT_FP_LESS(current, 0.15);

}

test_t t1 = {.name = "read voltage", .fn = read_voltage_test};
test_t t2 = {.name = "read current", .fn = read_current_test};
test_t t3 = {.name = "read temp", .fn = read_temp_test};
test_t t4 = {.name = "test heater", .fn = heater_test};
test_t t5 = {.name = "test imu", .fn = imu_test};
test_t t6 = {.name = "uptime test", .fn = uptime_test}
test_t t7 = {.name = "restart test", .fn = restart_test};
test_t t8 = {.name = "temporary low-power mode test", .fn = temp_low_power_mode_test};
test_t t9 = {.name = "indefinite low-power mode test", .fn = indefinite_low_power_mode_test};

test_t* suite[] = {&t1, &t2, &t3, &t4, &t5, &t6, &t7, &t8, &t9};

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

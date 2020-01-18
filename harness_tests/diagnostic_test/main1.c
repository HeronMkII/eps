/**
 * Diagnostic test to verify hardware functionality for EPS.
 *      - See Bus Architecture Specification (section 5- diagnostic tests) for more information
 **/

#include <stdbool.h>

#include <adc/adc.h>
#include <conversions/conversions.h>
#include <uart/uart.h>
#include <test/test.h>
#include <can/data_protocol.h>

#include "../../src/general.h"

#include "../../src/devices.h"
#include "../../src/heaters.h"
#include "../../src/measurements.h"
#include "../../src/imu.h"


uint8_t tx_msg[8] = {0x00};

/* Helper function for generating and appropriately dequeueing
    rx_housekeeping or rx_ctrl messages */
uint32_t construct_rx_msg(uint8_t op_code, uint8_t field_num, uint32_t tx_data){
    /* Initialize queue sizes to invalid valid */
    uint8_t rx_q_size = -1;
    uint8_t tx_q_size = -1;
    uint8_t rx_msg[8] = {0x00};

    rx_msg[2] = op_code;
    rx_msg[3] = field_num;
    tx_msg[4] = (tx_data >> 24) & 0xFF;
    tx_msg[5] = (tx_data >> 16) & 0xFF;
    tx_msg[6] = (tx_data >> 8) & 0xFF;
    tx_msg[7] = tx_data & 0xFF;

    enqueue(&can_rx_msg_queue, rx_msg);
    rx_q_size = queue_size(&can_rx_msg_queue);
    tx_q_size = queue_size(&can_tx_msg_queue);
    ASSERT_EQ(rx_q_size, 1);
    ASSERT_EQ(tx_q_size, 0);

    handle_rx_msg();
    rx_q_size = queue_size(&can_rx_msg_queue);
    tx_q_size = queue_size(&can_tx_msg_queue);
    ASSERT_EQ(rx_q_size, 0);
    ASSERT_EQ(tx_q_size, 1);

    dequeue(&can_tx_msg_queue, tx_msg);
    rx_q_size = queue_size(&can_rx_msg_queue);
    tx_q_size = queue_size(&can_tx_msg_queue);
    ASSERT_EQ(rx_q_size, 0);
    ASSERT_EQ(tx_q_size, 0);

    /* TODO: Check status bit to see if this is executed correctly */
    return tx_data;
}

/* Helper function for measuring heater currents */
double measure_heater_current(uint8_t source){
    construct_rx_msg(CAN_EPS_HK, source, 0x00);
    uint16_t raw_data = (tx_msg[4] << 8) & tx_msg[5];
    double current = adc_raw_to_circ_cur(raw_data, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF);
    return current;
}

/* Verifies that battery voltages are within valid range */
void read_voltage_test(void) {
    /* Battery voltage */
    /* Enqueue rx msg and handle */
    uint32_t raw_data_bat = construct_rx_msg(CAN_EPS_HK, CAN_EPS_HK_BAT_VOL, 0x00);

    /* Convert for assertion */
    double bat_voltage = adc_raw_to_circ_vol(raw_data_bat, ADC_VOL_SENSE_LOW_RES, ADC_VOL_SENSE_HIGH_RES);

    /* Assert that voltage is within range */
    ASSERT_FP_GREATER(bat_voltage, 4.0);
    ASSERT_FP_LESS(bat_voltage, 4.2);

    /* 3V3 Voltage */
    uint32_t raw_data_3v3 = construct_rx_msg(CAN_EPS_HK, CAN_EPS_HK_3V3_VOL, 0x00);
    double voltage_3v3 = adc_raw_to_circ_vol(raw_data_3v3, ADC_VOL_SENSE_LOW_RES, ADC_VOL_SENSE_HIGH_RES);

    /* Assert that voltage is within range */
    ASSERT_FP_GREATER(voltage_3v3, 3.29);
    ASSERT_FP_LESS(voltage_3v3, 3.31);

    /* 5V Voltage */
    uint32_t raw_data_5v = construct_rx_msg(CAN_EPS_HK, CAN_EPS_HK_5V_VOL, 0x00);
    double voltage_5v = adc_raw_to_circ_vol(raw_data_5v, ADC_VOL_SENSE_LOW_RES, ADC_VOL_SENSE_HIGH_RES);
    ASSERT_FP_GREATER(voltage_5v, 4.99);
    ASSERT_FP_LESS(voltage_5v, 5.01);
}

/* Verifies that battery and solar panel currents are within a valid range */
void read_current_test(void) {
    /* CAN_EPS_HK_BAT_CUR */
    uint32_t raw_data = construct_rx_msg(CAN_EPS_HK, CAN_EPS_HK_BAT_CUR, 0x00);
    double current = adc_raw_to_circ_cur(raw_data, ADC_BAT_CUR_SENSE_RES, ADC_BAT_CUR_SENSE_VREF);
    /* Assert current is within range */
    ASSERT_FP_GREATER(current, 0.2);
    ASSERT_FP_LESS(current, 0.3);

    /* CAN_EPS_HK_X_POS_CUR */
    raw_data = construct_rx_msg(CAN_EPS_HK, CAN_EPS_HK_X_POS_CUR, 0x00);
    current = adc_raw_to_circ_cur(raw_data, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF);
    /* Assert current is within range */
    ASSERT_FP_GREATER(current, 0.4);
    ASSERT_FP_LESS(current, 0.5);

    /* CAN_EPS_HK_X_NEG_CUR */
    raw_data = construct_rx_msg(CAN_EPS_HK, CAN_EPS_HK_X_NEG_CUR, 0x00);
    current = adc_raw_to_circ_cur(raw_data, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF);
    /* Assert current is within range */
    ASSERT_FP_GREATER(current, 0.4);
    ASSERT_FP_LESS(current, 0.5);

    /* CAN_EPS_HK_Y_POS_CUR */
    raw_data = construct_rx_msg(CAN_EPS_HK, CAN_EPS_HK_Y_POS_CUR, 0x00);
    current = adc_raw_to_circ_cur(raw_data, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF);
    /* Assert current is within range */
    ASSERT_FP_GREATER(current, 0.4);
    ASSERT_FP_LESS(current, 0.5);

    /* CAN_EPS_HK_Y_NEG_CUR */
    raw_data = construct_rx_msg(CAN_EPS_HK, CAN_EPS_HK_Y_NEG_CUR, 0x00);
    current = adc_raw_to_circ_cur(raw_data, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF);
    /* Assert current is within range */
    ASSERT_FP_GREATER(current, 0.4);
    ASSERT_FP_LESS(current, 0.5);

    /* CAN_EPS_HK_3V3_CUR */
    raw_data = construct_rx_msg(CAN_EPS_HK, CAN_EPS_HK_3V3_CUR, 0x00);
    current = adc_raw_to_circ_cur(raw_data, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF);
    /* Assert current is within range */
    ASSERT_FP_GREATER(current, 0.1);
    ASSERT_FP_LESS(current, 0.2);

    /* CAN_EPS_HK_5V_CUR */
    raw_data = construct_rx_msg(CAN_EPS_HK, CAN_EPS_HK_5V_CUR, 0x00);
    current = adc_raw_to_circ_cur(raw_data, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF);
    /* Assert current is within range */
    ASSERT_FP_GREATER(current, 0.2);
    ASSERT_FP_LESS(current, 0.3);

    /* CAN_EPS_HK_PAY_CUR */
    raw_data = construct_rx_msg(CAN_EPS_HK, CAN_EPS_HK_PAY_CUR, 0x00);
    current = adc_raw_to_circ_cur(raw_data, ADC_DEF_CUR_SENSE_RES, ADC_DEF_CUR_SENSE_VREF);
    /* Assert current is within range */
    ASSERT_FP_GREATER(current, 0.2);
    ASSERT_FP_LESS(current, 0.3);
}

/* Verifies that all temperatures are within valid range */
void read_temp_test(void) {
    uint16_t raw_data_temp = 0;
    double temp = 0;
    /* Battery 1 Temperature */
    raw_data_temp = construct_rx_msg(CAN_EPS_HK, CAN_EPS_HK_BAT_TEMP1, 0x00);
    temp = adc_raw_to_therm_temp(raw_data_temp);
    ASSERT_FP_GREATER(temp, 24.0);
    ASSERT_FP_LESS(temp, 26.0);

    /* Battery 2 Temperature */
    raw_data_temp = construct_rx_msg(CAN_EPS_HK, CAN_EPS_HK_BAT_TEMP2, 0x00);
    temp = adc_raw_to_therm_temp(raw_data_temp);
    ASSERT_FP_GREATER(temp, 24.0);
    ASSERT_FP_LESS(temp, 26.0);

    /* 3v3 Buck-Boost Converter Temperature */
    raw_data_temp = construct_rx_msg(CAN_EPS_HK, CAN_EPS_HK_3V3_TEMP, 0x00);
    temp = adc_raw_to_therm_temp(raw_data_temp);
    ASSERT_FP_GREATER(temp, 24.0);
    ASSERT_FP_LESS(temp, 26.0);

    /* 5V Boost Converter Temperature */
    raw_data_temp = construct_rx_msg(CAN_EPS_HK, CAN_EPS_HK_5V_TEMP, 0x00);
    temp = adc_raw_to_therm_temp(raw_data_temp);
    ASSERT_FP_GREATER(temp, 24.0);
    ASSERT_FP_LESS(temp, 26.0);

    /* Payload Connector Temperature */
    raw_data_temp = construct_rx_msg(CAN_EPS_HK, CAN_EPS_HK_PAY_CON_TEMP, 0x00);
    temp = adc_raw_to_therm_temp(raw_data_temp);
    ASSERT_FP_GREATER(temp, 24.0);
    ASSERT_FP_LESS(temp, 26.0);
}

/* Verifies that battery pack current increases by between 0.15A and 0.2A for each heater
    and between 0.3A and 0.4A for both heaters by first computing baseline current. Boost
    converter current should increase by 0.12-0.16 and 0.24-0.32A as above. */
void heater_test(void) {
    /* Initial values */
    double heater_off_curr_pack = measure_heater_current(CAN_EPS_HK_BAT_CUR);
    double heater_off_curr_boost = measure_heater_current(CAN_EPS_HK_5V_CUR);

    /* Turn heater 1 on */
    _delay_ms(1000);
    set_raw_heater_setpoint(&heater_1_shadow_setpoint, 0xFFF);
    set_raw_heater_setpoint(&heater_1_sun_setpoint, 0xFFF);

    /* Check current due to heater 1 on */
    _delay_ms(1000);
    double heater_on_curr_pack = measure_heater_current(CAN_EPS_HK_BAT_CUR);
    double heater_on_curr_boost = measure_heater_current(CAN_EPS_HK_5V_CUR);

    /* Assert correct ranges for battery back and boost converter */
    ASSERT_FP_GREATER(heater_on_curr_pack, heater_off_curr_pack + 0.15);
    ASSERT_FP_LESS(heater_on_curr_pack, heater_off_curr_pack + 0.2);

    ASSERT_FP_GREATER(heater_on_curr_boost, heater_off_curr_boost + 0.12);
    ASSERT_FP_LESS(heater_on_curr_boost, heater_off_curr_boost + 0.16);

    /* Turn heater 1 off */
    _delay_ms(1000);
    set_raw_heater_setpoint(&heater_1_shadow_setpoint, 0);
    set_raw_heater_setpoint(&heater_1_sun_setpoint, 0);

    /* Turn heater 2 on */
    set_raw_heater_setpoint(&heater_2_shadow_setpoint, 0xFFF);
    set_raw_heater_setpoint(&heater_2_sun_setpoint, 0xFFF);
    _delay_ms(1000);

    /* Check current due to heater 2 on */
    _delay_ms(1000);
    heater_on_curr_pack = measure_heater_current(CAN_EPS_HK_BAT_CUR);
    heater_on_curr_boost = measure_heater_current(CAN_EPS_HK_5V_CUR);

    /* Assert correct ranges */
    ASSERT_FP_GREATER(heater_on_curr_pack, heater_off_curr_pack + 0.15);
    ASSERT_FP_LESS(heater_on_curr_pack, heater_off_curr_pack + 0.2);

    ASSERT_FP_GREATER(heater_on_curr_boost, heater_off_curr_boost + 0.12);
    ASSERT_FP_LESS(heater_on_curr_boost, heater_off_curr_boost + 0.16);

    /* Turn heater 1 on */
    _delay_ms(1000);
    set_raw_heater_setpoint(&heater_1_shadow_setpoint, 0xFFF);
    set_raw_heater_setpoint(&heater_1_sun_setpoint, 0xFFF);

    // Check current due to both heaters on
    _delay_ms(1000);
    heater_on_curr_pack = measure_heater_current(CAN_EPS_HK_BAT_CUR);
    heater_on_curr_boost = measure_heater_current(CAN_EPS_HK_5V_CUR);

    /* Assert correct ranges */
    ASSERT_FP_GREATER(heater_on_curr_pack, heater_off_curr_pack + 0.3);
    ASSERT_FP_LESS(heater_on_curr_pack, heater_off_curr_pack + 0.4);

    ASSERT_FP_GREATER(heater_on_curr_boost, heater_off_curr_boost + 0.24);
    ASSERT_FP_LESS(heater_on_curr_boost, heater_off_curr_boost + 0.32);

    /* Turn heaters 1 and 2 off */
    set_raw_heater_setpoint(&heater_1_shadow_setpoint, 0);
    set_raw_heater_setpoint(&heater_1_sun_setpoint, 0);
    set_raw_heater_setpoint(&heater_2_shadow_setpoint, 0);
    set_raw_heater_setpoint(&heater_2_sun_setpoint, 0);
}

/* Tests calibrated and uncalibrated gyroscope values */
void imu_test(void) {
    uint16_t raw_data_imu = 0;
    double gyr_data = 0;
    uint8_t not_zero_flag = 0;

    for (int i=0; i<3; i++){
        raw_data_imu = construct_rx_msg(CAN_EPS_HK, CAN_EPS_HK_GYR_UNCAL_X, 0x00);
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
        raw_data_imu = construct_rx_msg(CAN_EPS_HK, CAN_EPS_HK_GYR_UNCAL_Y, 0x00);
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
        raw_data_imu = construct_rx_msg(CAN_EPS_HK, CAN_EPS_HK_GYR_UNCAL_Z, 0x00);
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
        raw_data_imu = construct_rx_msg(CAN_EPS_HK, CAN_EPS_HK_GYR_CAL_X, 0x00);
        gyr_data = imu_raw_data_to_gyro(raw_data_imu);
        ASSERT_FP_GREATER(gyr_data, -0.1);
        ASSERT_FP_LESS(gyr_data, 0.1);
        /* Ensures that at least one measurement is non-zero */
        if (gyr_data != 0){
            not_zero_flag |= 1;
        }
    }
    ASSERT_TRUE(not_zero_flag);

    for (int i=0; i<3; i++){
        raw_data_imu = construct_rx_msg(CAN_EPS_HK, CAN_EPS_HK_GYR_CAL_Y, 0x00);
        gyr_data = imu_raw_data_to_gyro(raw_data_imu);
        ASSERT_FP_GREATER(gyr_data, -0.1);
        ASSERT_FP_LESS(gyr_data, 0.1);
        /* Ensures that at least one measurement is non-zero */
        if (gyr_data != 0){
            not_zero_flag |= 1;
        }
    }
    ASSERT_TRUE(not_zero_flag);

    for (int i=0; i<3; i++){
        raw_data_imu = construct_rx_msg(CAN_EPS_HK, CAN_EPS_HK_GYR_CAL_Z, 0x00);
        gyr_data = imu_raw_data_to_gyro(raw_data_imu);
        ASSERT_FP_GREATER(gyr_data, -0.1);
        ASSERT_FP_LESS(gyr_data, 0.1);
        /* Ensures that at least one measurement is non-zero */
        if (gyr_data != 0){
            not_zero_flag |= 1;
        }
    }
    ASSERT_TRUE(not_zero_flag);
}

/* Sets current thresholds and asserts that heater has correct operational mode */
void heater_setpoint_test(void){
    construct_rx_msg(CAN_EPS_CTRL, CAN_EPS_CTRL_SET_HEAT_CUR_THRESH_LOWER, 10);
    construct_rx_msg(CAN_EPS_CTRL, CAN_EPS_CTRL_SET_HEAT_CUR_THRESH_UPPER, 10.05);
    control_heater_mode();
    ASSERT_EQ(heater_mode, HEATER_MODE_SHADOW);

    construct_rx_msg(CAN_EPS_CTRL, CAN_EPS_CTRL_SET_HEAT_CUR_THRESH_LOWER, 0);
    construct_rx_msg(CAN_EPS_CTRL, CAN_EPS_CTRL_SET_HEAT_CUR_THRESH_UPPER, 0.05);
    control_heater_mode();
    ASSERT_EQ(heater_mode, HEATER_MODE_SUN);
}

/* Asserts that the uptime value sent is within valid range */
void uptime_test(void){
    construct_rx_msg(CAN_EPS_HK, CAN_EPS_HK_UPTIME, 0x00);
    uint16_t tx_data = uptime_s;
    ASSERT_FP_GREATER(tx_data, 1); /* 1 second */
    ASSERT_FP_LESS(tx_data, 10); /* 10 seconds */
}

/* Asserts that the restart count value sent is within valid range */
void restart_test(void){
    construct_rx_msg(CAN_EPS_HK, CAN_EPS_HK_RESTART_COUNT, 0x00);
    uint16_t tx_data = restart_count;
    ASSERT_FP_GREATER(tx_data, 1);
    ASSERT_FP_LESS(tx_data, 1000);

    construct_rx_msg(CAN_EPS_HK, CAN_EPS_HK_RESTART_REASON, 0x00);
    tx_data = restart_reason;
    ASSERT_EQ(tx_data, 0x06);
}

/* Asserts that low power mode current is within valid range */
void temp_low_power_mode_test(void){
    /* start temporary low-power mode for 30 s */
    start_low_power_mode();

    /* CAN_EPS_HK_BAT_CUR */
    uint32_t raw_data = construct_rx_msg(CAN_EPS_HK, CAN_EPS_HK_BAT_CUR, 0x00);
    double current = adc_raw_to_circ_cur(raw_data, ADC_BAT_CUR_SENSE_RES, ADC_BAT_CUR_SENSE_VREF);
    /* Assert current is within range */
    ASSERT_FP_GREATER(current, 0.1);
    ASSERT_FP_LESS(current, 0.2);
}

/* Asserts that permanent low power mode current is within valid range */
void indefinite_low_power_mode_test(void){
  // Go to low power mode
  set_dac_raw_voltage(&dac, DAC_A, 0);
  set_dac_raw_voltage(&dac, DAC_B, 0);

  /* CAN_EPS_HK_BAT_CUR */
  uint32_t raw_data = construct_rx_msg(CAN_EPS_HK, CAN_EPS_HK_BAT_CUR, 0x00);
  double current = adc_raw_to_circ_cur(raw_data, ADC_BAT_CUR_SENSE_RES, ADC_BAT_CUR_SENSE_VREF);
  /* Assert current is within range */
  ASSERT_FP_GREATER(current, 0.1);
  ASSERT_FP_LESS(current, 0.15);

}

/* Returns current from solar panels for shunts test */
double get_shunts_data(uint8_t current_source){
    uint32_t raw_data = construct_rx_msg(CAN_EPS_HK, current_source, 0x00);
    double current = adc_raw_to_circ_cur(raw_data, ADC_BAT_CUR_SENSE_RES, ADC_BAT_CUR_SENSE_VREF);
    return current;
}

//TODO: Verify total battery current
/* Verifies that shunt currents are within valid range */
void shunts_test(void){
    turn_shunts_on();

    double current = get_shunts_data(CAN_EPS_HK_X_POS_CUR);
    ASSERT_FP_GREATER(current, 0);
    ASSERT_FP_LESS(current, 0.01);

    current = get_shunts_data(CAN_EPS_HK_X_NEG_CUR);
    ASSERT_FP_GREATER(current, 0);
    ASSERT_FP_LESS(current, 0.01);

    current = get_shunts_data(CAN_EPS_HK_Y_POS_CUR);
    ASSERT_FP_GREATER(current, 0);
    ASSERT_FP_LESS(current, 0.01);

    current = get_shunts_data(CAN_EPS_HK_Y_NEG_CUR);
    ASSERT_FP_GREATER(current, 0);
    ASSERT_FP_LESS(current, 0.01);
}

test_t t1 = {.name = "read voltage", .fn = read_voltage_test};
test_t t2 = {.name = "read current", .fn = read_current_test};
test_t t3 = {.name = "read temp", .fn = read_temp_test};
test_t t4 = {.name = "heater test", .fn = heater_test};
test_t t5 = {.name = "imu test", .fn = imu_test};
test_t t6 = {.name = "uptime test", .fn = uptime_test};
test_t t7 = {.name = "restart test", .fn = restart_test};
test_t t8 = {.name = "temporary low-power mode test", .fn = temp_low_power_mode_test};
test_t t9 = {.name = "indefinite low-power mode test", .fn = indefinite_low_power_mode_test};
test_t t10 = {.name = "heater setpoint test", .fn = heater_setpoint_test};
test_t t11 = {.name = "shunts current test", .fn = shunts_test};

test_t* suite[] = {&t1, &t2, &t3, &t4, &t5, &t6, &t7, &t8, &t9, &t10, &t11};

int main(void) {
    init_eps();
    /* Runs all tests in sequential order */
    run_tests(suite, sizeof(suite) / sizeof(suite[0]));
    return 0;
}

/*
NOTE: SET COOLTERM BAUD RATE TO 115,200
*/

#include "../../src/imu.h"

void receive_and_print_packets(uint16_t count) {
    for (uint16_t i = 0; i < count; i++) {
        if (!receive_imu_packet()) {
            print("Receive packet: FAIL\n");
            continue;
        }

        print("Receive packet: SUCCESS\n");
    }
}

void inf_loop_receive_and_print_packets(void) {
    while (1) {
        receive_and_print_packets(1);
    }
}

void test_seq_nums(void) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        print("seq_nums:");
        for (uint8_t i = 0; i < 6; i++) {
            print(" %u", imu_seq_nums[i]);
        }
        print("\n");
    }
}

void test_prod_id(void) {
    print("\nGetting product ID...\n\n");

    if (!get_imu_prod_id()) {
        print("\nGet product ID: FAIL\n\n");
        return;
    }

    print("\nGet product ID: SUCCESS\n\n");
}

void test_accel_once(void) {
    print("\nGetting acceleration...\n\n");

    int16_t x = 0, y = 0, z = 0;
    if (!get_imu_accel(&x, &y, &z)) {
        print("\nGet acceleration: FAIL\n\n");
        return;
    }

    print("\n");
    print("Get acceleration: SUCCESS\n");
    print("Raw: x = %d, y = %d, z = %d\n", x, y, z);
    print("Converted: x = %.6f, y = %.6f, z = %.6f\n",
        imu_raw_data_to_double(x, IMU_ACCEL_Q),
        imu_raw_data_to_double(y, IMU_ACCEL_Q),
        imu_raw_data_to_double(z, IMU_ACCEL_Q));
    print("\n");
}

void test_accel_inf(void) {
    while (1) {
        test_accel_once();
        _delay_ms(200);
    }
}

void test_accel_fast_once(void) {
    int16_t x = 0, y = 0, z = 0;
    get_imu_accel(&x, &y, &z);
    print("Acceleration: x = %+.6f, y = %+.6f, z = %+.6f\n",
        imu_raw_data_to_double(x, IMU_ACCEL_Q),
        imu_raw_data_to_double(y, IMU_ACCEL_Q),
        imu_raw_data_to_double(z, IMU_ACCEL_Q));
}

void test_uncal_gyro_once(void) {
    print("\nGetting uncalibrated gyroscope...\n\n");

    int16_t x = 0, y = 0, z = 0;
    int16_t bias_x = 0, bias_y = 0, bias_z = 0;
    if (!get_imu_uncal_gyro(&x, &y, &z, &bias_x, &bias_y, &bias_z)) {
        print("\nGet uncalibrated gyroscope: FAIL\n\n");
        return;
    }

    print("\n");
    print("Get uncalibrated gyroscope: SUCCESS\n");
    print("Raw: x = %d, y = %d, z = %d\n", x, y, z);
    print("Converted: x = %.6f, y = %.6f, z = %.6f\n",
        imu_raw_data_to_double(x, IMU_UNCAL_GYRO_Q),
        imu_raw_data_to_double(y, IMU_UNCAL_GYRO_Q),
        imu_raw_data_to_double(z, IMU_UNCAL_GYRO_Q));
    print("Raw: bias_x = %d, bias_y = %d, bias_z = %d\n", bias_x, bias_y, bias_z);
    print("Converted: bias_x = %.6f, bias_y = %.6f, bias_z = %.6f\n",
        imu_raw_data_to_double(bias_x, IMU_UNCAL_GYRO_Q),
        imu_raw_data_to_double(bias_y, IMU_UNCAL_GYRO_Q),
        imu_raw_data_to_double(bias_z, IMU_UNCAL_GYRO_Q));
    print("\n");
}

void test_uncal_gyro_inf(void) {
    while (1) {
        test_uncal_gyro_once();
        _delay_ms(500);
    }
}

void test_uncal_gyro_fast_once(void) {
    int16_t x = 0, y = 0, z = 0;
    get_imu_uncal_gyro(&x, &y, &z, NULL, NULL, NULL);
    print("Uncalibrated Gyroscope: x = %+.6f, y = %+.6f, z = %+.6f\n",
        imu_raw_data_to_double(x, IMU_UNCAL_GYRO_Q),
        imu_raw_data_to_double(y, IMU_UNCAL_GYRO_Q),
        imu_raw_data_to_double(z, IMU_UNCAL_GYRO_Q));
}

void test_cal_gyro_once(void) {
    print("\nGetting calibrated gyroscope...\n\n");

    int16_t x = 0, y = 0, z = 0;
    if (!get_imu_cal_gyro(&x, &y, &z)) {
        print("\nGet calibrated gyroscope.: FAIL\n\n");
        return;
    }

    print("\n");
    print("Get calibrated gyroscope.: SUCCESS\n");
    print("Raw: x = %d, y = %d, z = %d\n", x, y, z);
    print("Converted: x = %.6f, y = %.6f, z = %.6f\n",
        imu_raw_data_to_double(x, IMU_CAL_GYRO_Q),
        imu_raw_data_to_double(y, IMU_CAL_GYRO_Q),
        imu_raw_data_to_double(z, IMU_CAL_GYRO_Q));
    print("\n");
}

void test_cal_gyro_inf(void) {
    while (1) {
        test_cal_gyro_once();
        _delay_ms(500);
    }
}

// Without success/fail print statements
void test_cal_gyro_fast_once(void) {
    int16_t x = 0, y = 0, z = 0;
    get_imu_cal_gyro(&x, &y, &z);
    print("Calibrated Gyroscope: x = %+.6f, y = %+.6f, z = %+.6f\n",
        imu_raw_data_to_double(x, IMU_CAL_GYRO_Q),
        imu_raw_data_to_double(y, IMU_CAL_GYRO_Q),
        imu_raw_data_to_double(z, IMU_CAL_GYRO_Q));
}

// Compare uncalibrated with calibrated
void test_gyro_comp_inf(void) {
    print("\nGetting gyroscope...\n\n");

    while (1) {
        int16_t uncal_x = 0, uncal_y = 0, uncal_z = 0;
        int16_t bias_x = 0, bias_y = 0, bias_z = 0;
        get_imu_uncal_gyro(&uncal_x, &uncal_y, &uncal_z, &bias_x, &bias_y, &bias_z);
        int16_t cal_x = 0, cal_y = 0, cal_z = 0;
        get_imu_cal_gyro(&cal_x, &cal_y, &cal_z);

        print("\n");
        print("Uncalibrated: x = %+.6f, y = %+.6f, z = %+.6f\n",
            imu_raw_data_to_double(uncal_x, IMU_UNCAL_GYRO_Q),
            imu_raw_data_to_double(uncal_y, IMU_UNCAL_GYRO_Q),
            imu_raw_data_to_double(uncal_z, IMU_UNCAL_GYRO_Q));
        print("Calibrated:   x = %+.6f, y = %+.6f, z = %+.6f\n",
            imu_raw_data_to_double(cal_x, IMU_CAL_GYRO_Q),
            imu_raw_data_to_double(cal_y, IMU_CAL_GYRO_Q),
            imu_raw_data_to_double(cal_z, IMU_CAL_GYRO_Q));
        print("Bias:         x = %+.6f, y = %+.6f, z = %+.6f\n",
            imu_raw_data_to_double(bias_x, IMU_UNCAL_GYRO_Q),
            imu_raw_data_to_double(bias_y, IMU_UNCAL_GYRO_Q),
            imu_raw_data_to_double(bias_z, IMU_UNCAL_GYRO_Q));
    }
}


// Preferably disable IMU_DEBUG for this one
void test_all_fast_inf(void) {
    test_prod_id();

    while (1) {
        print("\n");
        test_accel_fast_once();
        test_uncal_gyro_fast_once();
        test_cal_gyro_fast_once();
        _delay_ms(500);
    }
}

int main(void) {
    init_uart();
    init_spi();

    // Use faster UART to interfere less with timing
    // This is necessary for the IMU to work!!
    set_uart_baud_rate(UART_BAUD_115200);

    print("\n\n");
    print("Starting IMU test\n");

    // This should initialize interrupts and receive the SHTP advertisement
    init_imu();

    // test_seq_nums();

    // test_prod_id();

    // test_accel_once();
    // receive_and_print_packets(5);
    // test_accel_inf();

    // test_uncal_gyro_once();
    // receive_and_print_packets(5);
    // test_uncal_gyro_inf();

    // test_cal_gyro_once();
    // receive_and_print_packets(5);
    // test_cal_gyro_inf();

    // test_cal_gyro_fast();

    // test_gyro_comp_inf();

    test_all_fast_inf();

    inf_loop_receive_and_print_packets();
    print("Done, looping...\n");
    while (1) {}
}

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

    test_seq_nums();
    test_prod_id();
    receive_and_print_packets(5);
    test_accel_inf();

    inf_loop_receive_and_print_packets();
    print("Done, looping...\n");
    while (1) {}
}

#include "../../src/imu.h"

void print_seq_nums(void) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        print("seq_nums:");
        for (uint8_t i = 0; i < 6; i++) {
            print(" %u", imu_seq_nums[i]);
        }
        print("\n");
    }
}

void print_prod_id(void) {
    print("\nGetting product ID...\n\n");

    if (!get_imu_prod_id()) {
        print("\nGet product ID: FAIL\n\n");
        return;
    }

    print("\nGet product ID: SUCCESS\n\n");
}

void test_accel(void) {
    print("\nGetting acceleration...\n\n");

    uint16_t x, y, z;
    if (!get_imu_accel(&x, &y, &z)) {
        print("\nGet acceleration: FAIL\n\n");
        return;
    }

    print("\nGet acceleration: SUCCESS\n\n");
}

int main(void) {
    init_uart();
    init_spi();

    // Use faster UART to interfere less with timing
    set_uart_baud_rate(UART_BAUD_115200);

    print("\n\n");
    print("Starting IMU test\n");

    // This should initialize interrupts and receive the SHTP advertisement
    init_imu();
    inf_loop_imu_receive();

    print_seq_nums();
    print_prod_id();
    test_accel();

    uint8_t ret = 0;
    uint16_t accel_x = 0;
    uint16_t accel_y = 0;
    uint16_t accel_z = 0;
    ret = get_imu_accel(&accel_x, &accel_y, &accel_z);
    print("get_imu_accel: ret = %u, x = %u, y = %u, z = %u\n", ret, accel_x, accel_y, accel_z);

    print("Done, looping...\n");
    while (1) {}
}

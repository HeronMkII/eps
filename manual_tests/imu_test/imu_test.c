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

int main(void) {
    init_uart();
    init_spi();

    print("\n\n");
    print("Starting IMU test\n");

    // This should initialize interrupts and receive the SHTP advertisement
    init_imu();
    print_seq_nums();

    uint8_t ret = 0;
    uint16_t accel_x = 0;
    uint16_t accel_y = 0;
    uint16_t accel_z = 0;
    ret = get_imu_accel(&accel_x, &accel_y, &accel_z);
    print("get_imu_accel: ret = %u, x = %u, y = %u, z = %u\n", ret, accel_x, accel_y, accel_z);

    print("Done, looping...\n");
    while (1) {}
}

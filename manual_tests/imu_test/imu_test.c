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


    print_seq_nums();
    // This should initialize interrupts and receive the SHTP advertisement
    init_imu();
    print_seq_nums();

    while (1) {}
}

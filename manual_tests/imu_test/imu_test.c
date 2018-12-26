#include "../../src/imu.h"

int main(void) {
    init_uart();
    init_spi();

    // This should initialize interrupts and receive the SHTP advertisement
    init_imu();

    while (1) {}
}

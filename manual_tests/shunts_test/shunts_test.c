#include <uart/uart.h>
#include <pex/pex.h>

#include "../../src/shunts.h"

void print_cmds(void) {
    print("1. Turn shunts on (battery charging off)\n");
    print("2. Turn shunts off (battery charging on)\n");
    print("3. Control shunts (based on threshold)\n");
}

uint8_t uart_cb(const uint8_t* data, uint8_t len) {
    switch (data[0]) {
        case 'h':
            print_cmds();
            break;
        case '1':
            turn_shunts_on();
            print("Turned shunts on\n");
            break;
        case '2':
            turn_shunts_off();
            print("Turned shunts off\n");
            break;
        case '3':
            control_shunts();
            print("Controlled shunts\n");
            break;
        default:
            break;
    }

    print("are_shunts_on = %u\n", are_shunts_on);

    return 1;
}

// This test reads the raw data and voltages on each ADC channel
// It converts the raw voltages to actual voltages and currents
int main(void) {
    init_uart();
    print("\n\nUART initialized\n");

    init_spi();
    print("SPI Initialized\n");

    init_shunts();
    print("Shunts Initialized\n");

    print("\nStarting test\n\n");
    print_cmds();
    print("\nPress 'h' to list commands again\n\n");

    set_uart_rx_cb(uart_cb);

    // Loop forever, the UART callback will interrupt
    while(1) {}
}

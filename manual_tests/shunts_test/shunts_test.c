#include <uart/uart.h>
#include <pex/pex.h>

#include "../../src/shunts.h"

bool automatic = false;

void print_cmds(void) {
    print("1. Turn shunts on (battery charging off)\n");
    print("2. Turn shunts off (battery charging on)\n");
    print("3. Run shunt control algorithm once\n");
    print("4. Enable automatic shunt control algorithm\n");
    print("5. Disable automatic shunt control algorithm\n");
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
            print("Ran shunt control algorithm\n");
            break;
        case '4':
            automatic = true;
            print("Enabled automatic\n");
            break;
        case '5':
            automatic = false;
            print("Disabled automatic\n");
            break;
        default:
            break;
    }

    print("are_shunts_on = %u\n", are_shunts_on);
    print("automatic = %u\n", automatic);

    return 1;
}

// This test reads the raw data and voltages on each ADC channel
// It converts the raw voltages to actual voltages and currents
int main(void) {
    init_uart();
    print("\n\nUART initialized\n");

    // Useful for seeing cause of reset
    print("MCUSR = 0x%.2x\n", MCUSR);
    MCUSR = 0;  // Seems like we need to do this to clear bits
    print("MCUSR = 0x%.2x\n", MCUSR);

    init_spi();
    print("SPI Initialized\n");

    // Make sure the ADC is initialized for battery voltage measurements
    init_adc(&adc);
    // Make sure PEX is initialized for shunt control
    init_pex(&pex);

    // Can change these thresholds for testing depending on the battery voltage
    shunts_on_threshold = 3.9;
    shunts_off_threshold = 3.8;

    init_shunts();
    print("Shunts Initialized\n");

    print("On PEX: -Y = A3, +Y = A2, -X = A1, +X = A0\n");

    print("\nStarting test\n\n");
    print_cmds();
    print("\nPress 'h' to list commands again\n\n");

    set_uart_rx_cb(uart_cb);

    // Loop forever, the UART callback will interrupt
    while(1) {
        if (automatic) {
            // Run the shunt algorithm
            control_shunts();
        }

        _delay_ms(1000);
    }
}

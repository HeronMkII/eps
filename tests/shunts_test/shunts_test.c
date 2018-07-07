#ifndef F_CPU
#define F_CPU 8000000L
#endif

#include <uart/uart.h>
#include <pex/pex.h>
#include <util/delay.h>
#include "../../src/shunts.h"

// This test reads the raw data and voltages on each ADC channel
// It converts the raw voltages to actual voltages and currents
int main(void) {
    init_uart();
    print("\n\nUART initialized\n");

    init_spi();
    print("SPI Initialized\n");

    pex_init_constants(PEX_EPS);
    pex_init();
    print("PEX Initialized\n");

    init_shunts();
    print("Shunts Initialized\n");

    print("\nStarting test\n\n");

    while(1){
        turn_shunts_on();
        print("Turned shunts on\n");

        for (uint16_t i = 0; i < 30; i++) {
            _delay_ms(1000);
        }

        turn_shunts_off();
        print("Turned shunts off\n");

        for (uint16_t i = 0; i < 30; i++) {
            _delay_ms(1000);
        }

        print("\n");
    }
}

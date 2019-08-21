/*
Test heaters low power mode
*/

#include <stdbool.h>

#include <adc/adc.h>
#include <uart/uart.h>
#include <uptime/uptime.h>

#include "../../src/devices.h"
#include "../../src/heaters.h"
#include "../../src/measurements.h"


uint8_t count = 0;
// Counts down for wait
void heaters_countdown(void) {
    print("Heater's timer: %d\n", count);
    if (count > 0) {
        count -= 1;
    }
}

int main(void) {
    init_uart();

    init_dac(&dac);
    init_adc(&adc);

    print("\n\n\nStarting test\n\n");

    init_uptime();
    init_heaters();
    add_uptime_callback(heaters_countdown);

    while (1) {
        print("\nStarting low power mode\n");
        start_low_power_mode();
        count = 30;
        _delay_ms(30000);
        print("\nLow power mode off\n");
        count = 5;
        _delay_ms(5000);
    }

    return 0;
}

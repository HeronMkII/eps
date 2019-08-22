/*
Test heaters low power mode
*/

#include <stdbool.h>

#include <adc/adc.h>
#include <uart/uart.h>
#include <uptime/uptime.h>
#include <conversions/conversions.h>

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

    // Turn heaters on
    double resistance = therm_temp_to_res(30);
    double voltage = therm_res_to_vol(resistance);
    set_dac_raw_voltage(&dac, DAC_A, voltage);
    set_dac_raw_voltage(&dac, DAC_B, voltage);

    while (1) {
        print("\nStarting low power mode\n");
        start_low_power_mode();
        count = HEATER_LOW_POWER_TIMER;
        _delay_ms(count * 1000);
        print("\nLow power mode off\n");
        count = 10;
        _delay_ms(count * 1000);
    }

    return 0;
}

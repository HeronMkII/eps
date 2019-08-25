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

    // Turn heaters on
    set_raw_heater_setpoint(&heater_1_shadow_setpoint,
        heater_setpoint_to_dac_raw_data(30));
    set_raw_heater_setpoint(&heater_2_shadow_setpoint,
        heater_setpoint_to_dac_raw_data(30));
    set_raw_heater_setpoint(&heater_1_sun_setpoint,
        heater_setpoint_to_dac_raw_data(10));
    set_raw_heater_setpoint(&heater_2_sun_setpoint,
        heater_setpoint_to_dac_raw_data(10));

    add_uptime_callback(heaters_countdown);

    while (1) {
        print("\nLow power mode off\n");
        count = 10;
        _delay_ms(count * 1000);
        print("\nStarting low power mode\n");
        start_low_power_mode();
        count = HEATER_LOW_POWER_TIMER;
        _delay_ms(count * 1000);
    }

    return 0;
}

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
    bool are_shunts_on_saved;

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
            are_shunts_on_saved = are_shunts_on;
            control_shunts();
            print("Ran shunt control algorithm\n");
            if (are_shunts_on == are_shunts_on_saved) {
                print("Shunts stayed the same\n");
            } else {
                print("Shunts changed\n");
            }
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

    // Set the IMU CSn (PD0) high (because it doesn't have a pullup resistor)
    // so it doesn't interfere with the PEX's output on the MISO line
    init_cs(PD0, &DDRD);
    set_cs_high(PD0, &PORTD);

    init_spi();
    print("SPI Initialized\n");

    init_shunts();
    print("Shunts Initialized\n");

    print("-Y = A3, +Y = A2, -X = A1, +X = A0\n");

    print("\nStarting test\n\n");
    print_cmds();
    print("\nPress 'h' to list commands again\n\n");

    set_uart_rx_cb(uart_cb);

    // Loop forever, the UART callback will interrupt
    while(1) {
        // Read battery voltage
        uint8_t channel = MEAS_PACK_VOUT;
        fetch_adc_channel(&adc, channel);
        uint16_t raw_data = read_adc_channel(&adc, channel);
        double batt_voltage = adc_raw_to_circ_vol(raw_data, ADC_VOL_SENSE_LOW_RES, ADC_VOL_SENSE_HIGH_RES);
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            print("Battery Voltage: %.6f V\n", batt_voltage);
        }

        if (automatic) {
            // Run the shunt algorithm and check if the state changed
            bool are_shunts_on_saved = are_shunts_on;
            control_shunts();
            if (are_shunts_on != are_shunts_on_saved) {
                print("Shunts changed\n");
                if (are_shunts_on) {
                    print("Shunts ON (charging OFF)\n");
                } else {
                    print("Shunts OFF (charging ON)\n");
                }
            }
        }

        _delay_ms(1000);
    }
}

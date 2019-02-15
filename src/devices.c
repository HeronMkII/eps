#include "devices.h"

// ADC
pin_info_t adc_cs = {
    .port = &ADC_CS_PORT_EPS,
    .ddr = &ADC_CS_DDR_EPS,
    .pin = ADC_CS_PIN_EPS
};
adc_t adc = {
    .channels = 0x0fff, // poll channels 0-11 in auto-1 mode
    .cs = &adc_cs
};

// PEX
pin_info_t pex_cs = {
    .port = &PEX_CS_PORT_EPS,
    .ddr = &PEX_CS_DDR_EPS,
    .pin = PEX_CS_PIN_EPS
};
pin_info_t pex_rst = {
    .port = &PEX_RST_PORT_EPS,
    .ddr = &PEX_RST_DDR_EPS,
    .pin = PEX_RST_PIN_EPS
};
pex_t pex = {
    .addr = PEX_ADDR_EPS,
    .cs = &pex_cs,
    .rst = &pex_rst
};

// DAC
// TODO - make constants
pin_info_t dac_cs = {
    .pin = PB4,
    .ddr = &DDRB,
    .port = &PORTB
};
pin_info_t dac_clr = {
    .pin = PC7,
    .ddr = &DDRC,
    .port = &PORTC
};
dac_t dac = {
    .cs = &dac_cs,
    .clr = &dac_clr,
    .raw_voltage_a = 0,
    .raw_voltage_b = 0
};

#ifndef DEVICES_H
#define DEVICES_H

#include <adc/adc.h>
#include <dac/dac.h>
#include <pex/pex.h>

// ADC CS
#define ADC_CS_PIN  PB2
#define ADC_CS_PORT PORTB
#define ADC_CS_DDR  DDRB

// DAC CS
#define DAC_CS_PIN  PB4
#define DAC_CS_PORT PORTB
#define DAC_CS_DDR  DDRB

// DAC CLR
#define DAC_CLR_PIN  PC7
#define DAC_CLR_PORT PORTC
#define DAC_CLR_DDR  DDRC

// PEX CS
#define PEX_CS_PIN  PB5
#define PEX_CS_PORT PORTB
#define PEX_CS_DDR  DDRB

// PEX RST
#define PEX_RST_PIN  PB4
#define PEX_RST_PORT PORTB
#define PEX_RST_DDR  DDRB

// PEX address
// TODO
#define PEX_ADDR 0

extern adc_t adc;
extern pex_t pex;
extern dac_t dac;

#endif

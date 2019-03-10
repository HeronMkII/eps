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
#define PEX_CS_PIN  PC1
#define PEX_CS_PORT PORTC
#define PEX_CS_DDR  DDRC

// PEX RST
#define PEX_RST_PIN  PD1
#define PEX_RST_PORT PORTD
#define PEX_RST_DDR  DDRD

// PEX address
#define PEX_ADDR 0b001

extern adc_t adc;
extern pex_t pex;
extern dac_t dac;

#endif

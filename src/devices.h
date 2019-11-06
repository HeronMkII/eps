#ifndef DEVICES_H
#define DEVICES_H

#include <adc/adc.h>
#include <dac/dac.h>
#include <pex/pex.h>

// ADC CS
#define ADC_CS_PIN  PB2
#define ADC_CS_PORT PORTB
#define ADC_CS_DDR  DDRB

// TODO - correct numbers

// Current sense resistor values (in ohms)
#define ADC_DEF_CUR_SENSE_RES   0.008
#define ADC_BAT_CUR_SENSE_RES   0.002

// Voltage references for INA214's (in V)
#define ADC_DEF_CUR_SENSE_VREF  0.0
#define ADC_BAT_CUR_SENSE_VREF  2.5

// Voltage divider resistor values (in ohms)
#define ADC_VOL_SENSE_LOW_RES   1e4
#define ADC_VOL_SENSE_HIGH_RES  1e4

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

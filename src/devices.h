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

#define ADC_THM_BATT2       0
#define ADC_THM_BATT1       1
#define ADC_THM_PAY_CONN    2
#define ADC_VMON_5V         3
#define ADC_IMON_X_PLUS     4
#define ADC_IMON_X_MINUS    5
#define ADC_IMON_Y_PLUS     6
#define ADC_IMON_Y_MINUS    7
#define ADC_IMON_5V         8
#define ADC_THM_5V_TOP      9
#define ADC_VMON_3V3        10
#define ADC_IMON_3V3        11
#define ADC_THM_3V3_TOP     12
#define ADC_VMON_PACK       13
#define ADC_IMON_PACK       14
#define ADC_IMON_PAY_LIM    15

extern adc_t adc;
extern pex_t pex;
extern dac_t dac;

#endif

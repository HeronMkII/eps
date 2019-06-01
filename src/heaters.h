#ifndef HEATERS_H
#define HEATERS_H

#include <dac/dac.h>
#include <avr/eeprom.h>

#include "devices.h"

#define EEPROM_DEF_DWORD 0xFFFFFFFF

//EEPROM address for storing raw data
#define HEATER_1_RAW_SETPOINT_ADDR ((uint32_t*) 0x70)
#define HEATER_2_RAW_SETPOINT_ADDR ((uint32_t*) 0x74)
void set_heater_1_raw_setpoint(uint16_t raw_data);
void set_heater_2_raw_setpoint(uint16_t raw_data);

void set_heater_1_temp_setpoint(double temp);
void set_heater_2_temp_setpoint(double temp);
void sunorshadow_setpoint();

void init_heaters();

#endif

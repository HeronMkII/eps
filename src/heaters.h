#ifndef HEATERS_H
#define HEATERS_H

#include <dac/dac.h>

#include "devices.h"

void set_heater_1_raw_setpoint(uint16_t raw_data);
void set_heater_2_raw_setpoint(uint16_t raw_data);

void set_heater_1_setpoint_temp(double temp);
void set_heater_2_setpoint_temp(double temp);

#endif

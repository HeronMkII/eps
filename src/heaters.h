#ifndef HEATERS_H
#define HEATERS_H

#include <dac/dac.h>
#include <avr/eeprom.h>

#include "devices.h"

//EEPROM address for storing raw data
#define HEATER_1_RAW_SHADOW_SETPOINT_ADDR   ((uint32_t*) 0x70)
#define HEATER_2_RAW_SHADOW_SETPOINT_ADDR   ((uint32_t*) 0x74)
#define HEATER_1_RAW_SUN_SETPOINT_ADDR      ((uint32_t*) 0x78)
#define HEATER_2_RAW_SUN_SETPOINT_ADDR      ((uint32_t*) 0x7C)

#define HEATER_1_STANDARD_SHADOW_SETPOINT   20   //Celsius
#define HEATER_2_STANDARD_SHADOW_SETPOINT   20   //Celsius
#define HEATER_1_STANDARD_SUN_SETPOINT      5    //Celsius
#define HEATER_2_STANDARD_SUN_SETPOINT      5    //Celsius

typedef enum {
    HEATER_MODE_SHADOW,
    HEATER_MODE_SUN
} heater_mode_t;


// Setpoints for different shadow/sun conditions (in raw 12-bit format)
extern uint16_t heater_1_raw_shadow_setpoint;
extern uint16_t heater_2_raw_shadow_setpoint;
extern uint16_t heater_1_raw_sun_setpoint;
extern uint16_t heater_2_raw_sun_setpoint;

extern double heater_sun_cur_thresh_upper;
extern double heater_sun_cur_thresh_lower;

extern heater_mode_t heater_mode;


void init_heaters(void);

void set_heater_1_raw_shadow_setpoint(uint16_t raw_data);
void set_heater_2_raw_shadow_setpoint(uint16_t raw_data);
void set_heater_1_raw_sun_setpoint(uint16_t raw_data);
void set_heater_2_raw_sun_setpoint(uint16_t raw_data);

uint16_t heater_setpoint_temp_to_raw(double temp);
double read_eps_cur(uint8_t channel);

void update_heater_setpoints(void);
void control_heater_mode(void);

#endif

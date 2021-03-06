#ifndef HEATERS_H
#define HEATERS_H

#include <dac/dac.h>
#include <avr/eeprom.h>

#include "devices.h"

//EEPROM address for storing raw data
#define HEATER_1_SHADOW_SETPOINT_ADDR   0x70
#define HEATER_2_SHADOW_SETPOINT_ADDR   0x74
#define HEATER_1_SUN_SETPOINT_ADDR      0x78
#define HEATER_2_SUN_SETPOINT_ADDR      0x7C
#define HEATER_CUR_THRESH_UPPER_ADDR    0x80
#define HEATER_CUR_THRESH_LOWER_ADDR    0x84

// Default setpoints (raw 12-bit DAC values)
#define HEATER_1_DEF_SHADOW_SETPOINT    0x400   // 25 C
#define HEATER_2_DEF_SHADOW_SETPOINT    0x400   // 25 C
#define HEATER_1_DEF_SUN_SETPOINT       0x39D   // 20 C
#define HEATER_2_DEF_SUN_SETPOINT       0x39D   // 20 C

// Default current thresholds (raw 12-bit ADC values)
#define HEATER_SUN_CUR_THRESH_UPPER     0x28F   // 1 A
#define HEATER_SUN_CUR_THRESH_LOWER     0x26E   // 0.95 A

#define HEATER_CTRL_PERIOD_S 60


typedef struct {
    // Raw 12-bit DAC format
    uint16_t raw;
    uint16_t eeprom_addr;
} heater_val_t;

typedef enum {
    HEATER_MODE_SHADOW,
    HEATER_MODE_SUN
} heater_mode_t;


// Setpoints for different shadow/sun conditions (in raw 12-bit format)
extern heater_val_t heater_1_shadow_setpoint;
extern heater_val_t heater_2_shadow_setpoint;
extern heater_val_t heater_1_sun_setpoint;
extern heater_val_t heater_2_sun_setpoint;

extern heater_val_t heater_sun_cur_thresh_upper;
extern heater_val_t heater_sun_cur_thresh_lower;

extern heater_mode_t heater_mode;


void init_heaters(void);

void set_raw_heater_setpoint(heater_val_t* setpoint, uint16_t raw_data);
void set_raw_heater_cur_thresh(heater_val_t* cur_thresh, uint16_t raw_data);

void update_heater_setpoint_outputs(void);
void control_heater_mode(void);
void run_heaters(void);

#endif

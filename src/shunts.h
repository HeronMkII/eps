#ifndef SHUNTS_H
#define SHUNTS_H

#include <stdbool.h>
#include <pex/pex.h>
#include <adc/adc.h>
#include <conversions/conversions.h>


// If the battery output voltage goes above this threshold,
// the shunts should be turned on (battery charging off)
#define SHUNTS_ON_BATT_VOUT_THRESHOLD 4.0
// If the battery output voltage goes below this threshold,
// the shunts should be turned off (battery charging on)
#define SHUNTS_OFF_BATT_VOUT_THRESHOLD 3.0

extern bool are_shunts_on;

void init_shunts(void);
void turn_shunts_on(void);
void turn_shunts_off(void);
void control_shunts(void);

#endif

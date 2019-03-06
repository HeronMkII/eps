#ifndef SHUNTS_H
#define SHUNTS_H

#include <stdbool.h>

#include <adc/adc.h>
#include <conversions/conversions.h>
#include <pex/pex.h>
#include <uart/uart.h>

#include "devices.h"
#include "measurements.h"


// Shunt pins on PEX GPIOA (all on bank A)
// For battery charging (solar panel faces)
#define SHUNTS_POS_X 0   // +X face
#define SHUNTS_NEG_X 1   // -X face
#define SHUNTS_POS_Y 2   // +Y face
#define SHUNTS_NEG_Y 3   // -Y face

// If the battery output voltage goes above this threshold,
// the shunts should be turned on (battery charging off)
#define SHUNTS_ON_THRESHOLD 4.0
// If the battery output voltage goes below this threshold,
// the shunts should be turned off (battery charging on)
#define SHUNTS_OFF_THRESHOLD 3.0

extern bool are_shunts_on;

void init_shunts(void);
void turn_shunts_on(void);
void turn_shunts_off(void);
void control_shunts(void);

#endif

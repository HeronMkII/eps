#ifndef SHUNTS_H
#define SHUNTS_H

#include <stdbool.h>

#include <adc/adc.h>
#include <conversions/conversions.h>
#include <pex/pex.h>
#include <uart/uart.h>

#include "devices.h"


// Shunt pins on PEX GPIOA (all on bank A)
// For battery charging (solar panel faces)
#define SHUNTS_POS_X 0   // +X face
#define SHUNTS_NEG_X 1   // -X face
#define SHUNTS_POS_Y 2   // +Y face
#define SHUNTS_NEG_Y 3   // -Y face

// Default threshold values
#define SHUNTS_ON_DEF_THRESHOLD     4.19
#define SHUNTS_OFF_DEF_THRESHOLD    4.15

extern bool are_shunts_on;

extern double shunts_on_threshold;
extern double shunts_off_threshold;

void init_shunts(void);
void turn_shunts_on(void);
void turn_shunts_off(void);
void control_shunts(void);

#endif

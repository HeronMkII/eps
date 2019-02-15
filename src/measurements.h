/*
Pin constants for taking measurements. We don't need a .c file because the
implementation is in the lib-common adc library.
*/

#ifndef MEASUREMENTS_H
#define MEASUREMENTS_H

// ADC channels
// From EPS schematics, p.7, eps_digital.SchDoc

// Buck-boost converter output voltage (BBV)
#define MEAS_BB_VOUT    0
// Buck-boost converter output current (BBI)
#define MEAS_BB_IOUT    1
// Photovoltaic cell (-Y) output current (-YI)
#define MEAS_NEG_Y_IOUT 2
// Photovoltaic cell (+X) output current (+XI)
#define MEAS_POS_X_IOUT 3
// Photovoltaic cell (+Y) output current (+YI)
#define MEAS_POS_Y_IOUT 4
// Photovoltaic cell (-X) output current (-XI)
#define MEAS_NEG_X_IOUT 5
// Battery pack thermistor 1 (THM_IN1)
#define MEAS_THERM_1    6
// Battery pack thermistor 2 (THM_IN2)
#define MEAS_THERM_2    7
// Battery pack positive voltage (VPACK/+PACK)
#define MEAS_PACK_VOUT  8
// Battery pack output current (IPACK)
#define MEAS_PACK_IOUT  9
// Boost converter output current (BTI)
#define MEAS_BT_IOUT    10
// Boost converter output voltage (BTV)
#define MEAS_BT_VOUT    11

#endif

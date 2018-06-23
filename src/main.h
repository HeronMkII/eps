#ifndef MAIN_H
#define MAIN_H

#include <utilities/utilities.h>

// Standard libraries
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// AVR libraries
#include <util/delay.h>

// lib-common libraries
#include <can/can.h>
#include <can/can_ids.h>
#include <can/can_data_protocol.h>
#include <queue/queue.h>
#include <spi/spi.h>
#include <uart/uart.h>
#include <adc/adc.h>
#include <pex/pex.h>

#include "shunts.h"
#include "can_callbacks.h"

void handle_rx(void);
void init_eps(void);

#endif

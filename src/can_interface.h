#ifndef CAN_INTERFACE_H
#define CAN_INTERFACE_H

#include <stdint.h>
#include <can/can.h>
#include <can/ids.h>
#include <can/data_protocol.h>
#include <queue/queue.h>
#include <uart/uart.h>

#include "can_commands.h"

// CAN MOBs
extern mob_t cmd_rx_mob;
extern mob_t data_tx_mob;

#endif

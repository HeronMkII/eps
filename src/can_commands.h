#ifndef CAN_COMMANDS_H
#define CAN_COMMANDS_H

#include <stdint.h>

#include <can/data_protocol.h>
#include <queue/queue.h>
#include <uart/uart.h>

#include "can_interface.h"
#include "devices.h"
#include "general.h"

extern queue_t can_rx_msgs;
extern queue_t can_tx_msgs;

void handle_rx_msg(void);

#endif

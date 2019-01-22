#ifndef CAN_COMMANDS_H
#define CAN_COMMANDS_H

#include <stdbool.h>
#include <stdint.h>

#include <can/data_protocol.h>
#include <queue/queue.h>
#include <uart/uart.h>

#include "can_interface.h"
#include "devices.h"
#include "general.h"

extern queue_t can_rx_msg_queue;
extern queue_t can_tx_msg_queue;

extern bool sim_local_actions;

void handle_rx_msg(void);

#endif

#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdint.h>
#include <queue/queue.h>
#include <uart/uart.h>
#include <can/data_protocol.h>
#include "can_interface.h"
#include "eps.h"

void handle_rx_msg(void);

#endif

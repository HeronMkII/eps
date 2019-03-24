#ifndef GENERAL_H
#define GENERAL_H

#include <can/can.h>
#include <queue/queue.h>
#include <spi/spi.h>
#include <watchdog/watchdog.h>

#include "can_commands.h"
#include "can_interface.h"
#include "shunts.h"


void init_eps(void);
void process_next_rx_msg(void);
void send_next_tx_msg(void);

#endif

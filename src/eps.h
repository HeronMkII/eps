#ifndef EPS_H
#define EPS_H

#include <can/can.h>
#include <queue/queue.h>
#include <spi/spi.h>

#include "can_interface.h"
#include "shunts.h"
#include "commands.h"

extern adc_t adc;
extern pex_t pex;

void init_eps(void);
void process_next_rx_msg(void);
void send_next_tx_msg(void);

#endif

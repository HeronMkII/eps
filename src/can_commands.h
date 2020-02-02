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
#include "heaters.h"
#include "imu.h"

extern queue_t can_rx_msg_queue;
extern queue_t can_tx_msg_queue;

void process_next_rx_msg(void);
void send_next_tx_msg(void);

#endif

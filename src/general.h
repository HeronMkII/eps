#ifndef GENERAL_H
#define GENERAL_H

#include <can/can.h>
#include <heartbeat/heartbeat.h>
#include <queue/queue.h>
#include <spi/spi.h>
#include <uptime/uptime.h>
#include <watchdog/watchdog.h>

#include "can_commands.h"
#include "can_interface.h"
#include "shunts.h"

void init_eps(void);

#endif

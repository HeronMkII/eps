// Definition necessary for <util/delay.h>
#ifndef F_CPU
#define F_CPU 8000000UL
#endif

// AVR
#include <util/delay.h>

// Standard libraries
#include <stdbool.h>
#include <stdint.h>

// lib-common libraries
#include <can/can.h>
#include <can/can_ids.h>
#include <can/can_data_protocol.h>
#include <queue/queue.h>
#include <spi/spi.h>
#include <uart/uart.h>
#include <adc/adc.h>
#include <pex/pex.h>
#include <utilities/utilities.h>

#include "shunts.h"



// CAN interrupts
void status_rx_callback(const uint8_t* data, uint8_t len);
void status_tx_callback(uint8_t* data, uint8_t* len);
void cmd_tx_callback(uint8_t* data, uint8_t* len);
void cmd_rx_callback(const uint8_t* data, uint8_t len);
void data_tx_callback(uint8_t* data, uint8_t* len);


// CAN MOBs

mob_t status_rx_mob = {
	.mob_num = 0,
	.mob_type = RX_MOB,
    .dlc = 8,
    .id_tag = PAY_STATUS_RX_MOB_ID,
	.id_mask = CAN_RX_MASK_ID,
    .ctrl = default_rx_ctrl,

    .rx_cb = status_rx_callback
};

mob_t status_tx_mob = {
    .mob_num = 1,
	.mob_type = TX_MOB,
    .id_tag = PAY_STATUS_TX_MOB_ID,
    .ctrl = default_tx_ctrl,

    .tx_data_cb = status_tx_callback
};

mob_t cmd_tx_mob = {
	.mob_num = 2,
	.mob_type = TX_MOB,
	.id_tag = PAY_CMD_TX_MOB_ID,
	.ctrl = default_tx_ctrl,

	.tx_data_cb = cmd_tx_callback
};

mob_t cmd_rx_mob = {
	.mob_num = 4,
	.mob_type = RX_MOB,
    .dlc = 8,
    .id_tag = PAY_CMD_RX_MOB_ID,
	// .id_mask = { 0x0000 },
	.id_mask = CAN_RX_MASK_ID,
    .ctrl = default_rx_ctrl,

    .rx_cb = cmd_rx_callback
};

mob_t data_tx_mob = {
    .mob_num = 5,
	.mob_type = TX_MOB,
    .id_tag = PAY_DATA_TX_MOB_ID,
    .ctrl = default_tx_ctrl,

    .tx_data_cb = data_tx_callback
};





void print_bytes(uint8_t *data, uint8_t len);
void handle_rx(void);
void init_eps(void);

#include "can_callbacks.h"





// CAN MOBs

// mob_t status_rx_mob = {
// 	.mob_num = 0,
// 	.mob_type = RX_MOB,
//     .dlc = 8,
//     .id_tag = PAY_STATUS_RX_MOB_ID,
// 	.id_mask = CAN_RX_MASK_ID,
//     .ctrl = default_rx_ctrl,
//
//     .rx_cb = status_rx_callback
// };

// mob_t status_tx_mob = {
//     .mob_num = 1,
// 	.mob_type = TX_MOB,
//     .id_tag = PAY_STATUS_TX_MOB_ID,
//     .ctrl = default_tx_ctrl,
//
//     .tx_data_cb = status_tx_callback
// };

// mob_t cmd_tx_mob = {
// 	.mob_num = 2,
// 	.mob_type = TX_MOB,
// 	.id_tag = PAY_CMD_TX_MOB_ID,
// 	.ctrl = default_tx_ctrl,
//
// 	.tx_data_cb = cmd_tx_callback
// };









/* CAN Interrupts */


// // MOB 0
// void status_rx_callback(const uint8_t* data, uint8_t len) {
//     print("MOB 0: Status RX Callback\n");
//     print("Received Message:\n");
//     print_hex_bytes((uint8_t *) data, len);
// }

// // MOB 1
// void status_tx_callback(uint8_t* data, uint8_t* len) {
//     print("MOB 1: Status TX Callback\n");
// }

// // MOB 2
// void cmd_tx_callback(uint8_t* data, uint8_t* len) {
//     print("MOB 2: CMD TX Callback\n");
// }

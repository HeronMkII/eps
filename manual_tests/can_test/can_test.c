/*
An outdated test for sending dummy messages (strings) to and from EPS over CAN.
*/

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <uart/uart.h>
#include <can/can.h>
#include <can/ids.h>
#include <can/data_protocol.h>
#include <util/delay.h>
#include <stdbool.h>

// Counter for which request to send (0-3)
uint8_t req_num = 3;            // current
const uint8_t NUM_REQ_NUMS = 4; // total number (count)

void tx_callback(uint8_t*, uint8_t*);
void rx_callback(const uint8_t*, uint8_t);

mob_t tx_mob = {
    .mob_num = 0,
    .mob_type = TX_MOB,
    .id_tag = OBC_PAY_CMD_TX_MOB_ID,
    .ctrl = default_tx_ctrl,

    .tx_data_cb = tx_callback
};

mob_t rx_mob = {
    .mob_num = 3,
    .mob_type = RX_MOB,
    .dlc = 8,
    .id_tag = OBC_DATA_RX_MOB_ID,
    .id_mask = CAN_RX_MASK_ID,
    .ctrl = default_rx_ctrl,

    .rx_cb = rx_callback
};


bool string_equal(uint8_t *array, char *string, uint8_t len) {
    for (uint8_t i = 0; i < len; i++) {
        if (array[i] != string[i]) {
            return false;
        }
    }

    return true;
}


void tx_callback(uint8_t* data, uint8_t* len) {
    print("\nTX Callback\n");

    *len = 8;
    for (uint8_t i = 0; i < 8; ++i) {
        data[i] = 0;
    }

    req_num = (req_num + 1) % NUM_REQ_NUMS;

    switch (req_num) {
        case 0:
            data[0] = 1;
            data[1] = 1;
            break;

        case 1:
            data[0] = 1;
            data[1] = 2;
            break;

        case 2:
            data[0] = 2;
            data[1] = 1;
            break;

        case 3:
            data[0] = 2;
            data[1] = 2;
            break;

        default:
            req_num = 0;
            break;
    }

    print("Transmitting Message:\n");
    print_bytes(data, *len);
}


void rx_callback(const uint8_t* data, uint8_t len) {
    print("RX Callback\n");
    print("Received Message:\n");
    print_bytes((uint8_t *) data, len);
    print("%s\n", (char *) data);

    // switch (req_num) {
    //     case 0:
    //         ASSERT(string_equal((uint8_t *) data, "hello11", 8));
    //         break;
    //
    //     case 1:
    //         ASSERT(string_equal((uint8_t *) data, "hello12", 8));
    //         break;
    //
    //     case 2:
    //         ASSERT(string_equal((uint8_t *) data, "hello21", 8));
    //         break;
    //
    //     case 3:
    //         ASSERT(string_equal((uint8_t *) data, "hello22", 8));
    //         break;
    //
    //     default:
    //         break;
    // }
}


int main(void) {
    init_uart();
    print("\n\nUART Initialized\n");

    init_can();
    init_tx_mob(&tx_mob);
    init_rx_mob(&rx_mob);
    print("CAN Initialized\n");

    while(1) {
        resume_mob(&tx_mob);
        while (!is_paused(&tx_mob)) {};
        _delay_ms(5000);
    }
}

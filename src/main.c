// Standard libraries
#include "eps.h"

int main(void) {
    init_eps();

    // Main loop (infinite)
    while (1) {
        // Process an RX CAN message
        process_next_rx_msg();
        // Process a TX CAN message
        send_next_tx_msg();
    }
}

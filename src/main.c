// Standard libraries
#include "general.h"

int main(void) {
    WDT_OFF();
    WDT_ENABLE_SYS_RESET(WDTO_8S);

    init_eps();

    // Main loop (infinite)
    while (1) {
        // Reset watchdog timer
        WDT_ENABLE_SYS_RESET(WDTO_8S);
        // Send a TX CAN message
        send_next_tx_msg();
        // Process an RX CAN message
        process_next_rx_msg();
    }
}

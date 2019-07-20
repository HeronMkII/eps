// Standard libraries
#include "general.h"

int main(void) {
    WDT_OFF();
    WDT_ENABLE_SYS_RESET(WDTO_8S);

    init_eps();
    init_hb(HB_EPS);

    // Main loop (infinite)
    while (1) {
        // Reset watchdog timer
        WDT_ENABLE_SYS_RESET(WDTO_8S);
        // Possibly send/receive heartbeat
        run_hb();
        // Shunt control algorithm
        control_shunts();
        // Send a TX CAN message
        send_next_tx_msg();
        // Process an RX CAN message
        process_next_rx_msg();
    }
}

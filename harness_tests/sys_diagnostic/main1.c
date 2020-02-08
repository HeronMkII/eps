#include <stdlib.h>
#include <test/test.h>
#include <uptime/uptime.h>

#include "../../src/general.h"

/* Resets the PAY SSM and verifies that the reset counter increments correctly */
void pay_reset_test(void){
    uint32_t stale_num_restarts = 0x00;

    pay_hb_dev.send_req_flag = true;
    _delay_ms(100);
    run_hb();
    _delay_ms(100);
    stale_num_restarts = pay_hb_dev.restart_count;

    send_hb_reset((hb_dev_t*) &pay_hb_dev);

    _delay_ms(1000);
    pay_hb_dev.send_req_flag = true;
    _delay_ms(100);
    run_hb();
    _delay_ms(100);

    ASSERT_EQ(pay_hb_dev.restart_count, stale_num_restarts + 1);
    ASSERT_EQ(pay_hb_dev.restart_reason, UPTIME_RESTART_REASON_EXTRF);
}

/* Resets the OBC SSM and verifies that the reset counter increments correctly */
void obc_reset_test(void){
    uint32_t stale_num_restarts = 0x00;

    obc_hb_dev.send_req_flag = true;
    _delay_ms(100);
    run_hb();
    _delay_ms(100);
    stale_num_restarts = obc_hb_dev.restart_count;

    send_hb_reset((hb_dev_t*) &obc_hb_dev);

    _delay_ms(1000);
    obc_hb_dev.send_req_flag = true;
    _delay_ms(100);
    run_hb();
    _delay_ms(100);

    ASSERT_EQ(obc_hb_dev.restart_count, stale_num_restarts + 1);
    ASSERT_EQ(obc_hb_dev.restart_reason, UPTIME_RESTART_REASON_EXTRF);
}

test_t t1 = { .name = "PAY Reset Test", .fn = pay_reset_test };
test_t t2 = { .name = "OBC Reset Test", .fn = obc_reset_test };

test_t* suite[] = { &t1, &t2};

int main(void) {
    init_eps();
    init_hb(HB_EPS);
    run_tests(suite, sizeof(suite) / sizeof(suite[0]));
    return 0;
}

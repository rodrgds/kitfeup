#include <stdio.h>

#include "timer/timer_irq.h"
#include "umdp.h"

int main(void) {
    umdp_connection* conn;
    timer_irq_ctx timer;
    int ret;

    conn = umdp_connect();
    if (conn == NULL) {
        return 1;
    }

    ret = timer_irq_init(&timer, conn, 200u, SG2000_TIMER_DEFAULT_IRQ);
    if (ret != 0) {
        umdp_disconnect(conn);
        return 1;
    }

    ret = timer_irq_wait_tick(&timer);
    timer_irq_stop(&timer);
    timer_irq_cleanup(&timer);
    umdp_disconnect(conn);

    if (ret != 0) {
        return 1;
    }

    printf("TIMER_PROBE_OK\n");
    return 0;
}

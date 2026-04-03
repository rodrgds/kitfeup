#include <errno.h>
#include <stdio.h>
#include <stdint.h>

#include "umdp.h"
#include "timer/timer_multi.h"

typedef struct {
    uint32_t id;
    uint32_t period_ms;
} timer_pair;

static const timer_pair demo_timers[] = {
    {1u, 250u},
    {2u, 1100u},
};

#define DEMO_TIMER_COUNT (sizeof(demo_timers) / sizeof(demo_timers[0]))

int main(void) {
    umdp_connection* conn = umdp_connect();
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to UMDP\n");
        return 1;
    }

    timer_multi_ctx multi;
    int ret = timer_multi_init(&multi, conn, 50u, SG2000_TIMER_DEFAULT_IRQ);
    if (ret != 0) {
        fprintf(stderr, "timer_multi_init failed: %s\n", umdp_strerror(ret));
        umdp_disconnect(conn);
        return 1;
    }

    for (size_t i = 0u; i < DEMO_TIMER_COUNT; i++) {
        ret = timer_multi_add(&multi, demo_timers[i].id, demo_timers[i].period_ms);
        if (ret != 0) {
            fprintf(stderr, "timer_multi_add(id=%u, period=%ums) failed: %s\n", demo_timers[i].id,
                demo_timers[i].period_ms, umdp_strerror(ret));
            timer_multi_cleanup(&multi);
            umdp_disconnect(conn);
            return 1;
        }
    }

    printf("=== Multi Timer Demo ===\n");
    printf("Base tick: 50 ms (hardware timer period)\n");
    for (size_t i = 0u; i < DEMO_TIMER_COUNT; i++) {
        printf("Timer %u: %u ms\n", demo_timers[i].id, demo_timers[i].period_ms);
    }
    printf("Waiting for events... Press Ctrl+C to stop\n\n");

    while (1) {
        uint32_t fired[8];
        size_t fired_count = 0u;

        ret = timer_multi_wait(&multi, fired, 8u, &fired_count);
        if (ret != 0) {
            if (ret == ETIMEDOUT) {
                fprintf(stderr, "[warn] No timer interrupt received within timeout window\n");
                continue;
            }
            fprintf(stderr, "timer_multi_wait failed: %s\n", umdp_strerror(ret));
            break;
        }

        for (size_t i = 0u; i < fired_count; i++) {
            bool found = false;
            for (size_t t = 0u; t < DEMO_TIMER_COUNT; t++) {
                if (demo_timers[t].id == fired[i]) {
                    printf("[T%u] fired (%ums)\n", fired[i], demo_timers[t].period_ms);
                    found = true;
                    break;
                }
            }
            if (!found) {
                printf("[T%u] fired\n", fired[i]);
            }
        }
    }

    timer_multi_cleanup(&multi);
    umdp_disconnect(conn);
    return 0;
}

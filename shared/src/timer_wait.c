/*
 * Timer Test Program for SG2000 timer IRQ path
 *
 * Demonstrates the use of the timer library to configure and wait for
 * hardware timer interrupts using UMDP.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include "umdp.h"
#include "timer/timer_irq.h"

/* Global state */
static volatile uint32_t tick_count = 0;

int main(int argc, char *argv[])
{
    umdp_connection *conn;
    timer_irq_ctx timer;
    int ret;
    uint32_t period_ms = 250;

    /* Parse command line arguments */
    if (argc > 1)
    {
        period_ms = atoi(argv[1]);
        if (period_ms == 0)
        {
            fprintf(stderr, "Invalid period: %s (using default 250ms)\n", argv[1]);
            period_ms = 250;
        }
    }

    printf("=== Timer Test Program ===\n");
    printf("Timer Frequency: %u Hz\n", SG2000_TIMER_FREQ_HZ);
    printf("Period: %u ms\n", period_ms);

    /* 1. Connect to UMDP */
    conn = umdp_connect();
    if (conn == NULL)
    {
        fprintf(stderr, "Failed to connect to UMDP\n");
        return 1;
    }
    printf("Connected to UMDP successfully\n");

    /* 2. Initialize timer */
    ret = timer_irq_init(&timer, conn, period_ms, SG2000_TIMER_DEFAULT_IRQ);
    if (ret != 0)
    {
        fprintf(stderr, "Failed to initialize timer: %s\n", umdp_strerror(ret));
        umdp_disconnect(conn);
        return 1;
    }
    printf("Timer initialized with %u ms period\n", period_ms);

    /* 3. Wait for timer interrupts */
    printf("\nWaiting for timer interrupts...\n\n");

    while (1)
    {
        ret = timer_irq_wait_tick(&timer);
        if (ret == 0)
        {
            tick_count++;
            printf("[%u] Interrupt received\n", tick_count);
        }
        else if (ret == ETIMEDOUT)
        {
            printf("[warn] No timer interrupt received within timeout window\n");
            continue;
        }
        else
        {
            fprintf(stderr, "Error waiting for timer interrupt: %s\n", umdp_strerror(ret));
            break;
        }
    }

    /* 4. Cleanup */
    printf("\n--- Timer Statistics ---\n");
    printf("Total timer ticks: %u\n", tick_count);

    timer_irq_stop(&timer);
    timer_irq_cleanup(&timer);
    umdp_disconnect(conn);
    printf("UMDP disconnected\n");

    return 0;
}

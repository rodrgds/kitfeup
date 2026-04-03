/*
 * Blink LED using timer_irq library (interrupt-driven)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include "umdp.h"
#include "timer/timer_irq.h"
#include "led/led_gpio.h"

#define TIMER_IRQ SG2000_TIMER_DEFAULT_IRQ

/* Global state */
static volatile bool running = true;
static volatile uint32_t toggle_count = 0;
static volatile uint32_t led_state = 0;

/* Signal handler */
static void signal_handler(int sig)
{
    (void)sig;
    running = false;
    printf("\nStopping...\n");
}

static int install_sigint_handler(void)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) != 0)
    {
        perror("sigaction");
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    umdp_connection *conn;
    int ret;
    uint32_t blink_period_ms = 250;
    timer_irq_ctx timer;
    led_gpio_ctx led;

    /* Parse arguments */
    if (argc > 1)
    {
        blink_period_ms = atoi(argv[1]);
        if (blink_period_ms == 0)
        {
            fprintf(stderr, "Invalid period: %s (using default 250ms)\n", argv[1]);
            blink_period_ms = 250;
        }
    }

    printf("=== Step 6: LED Blink using Timer Interrupts ===\n");
    printf("LED GPIO Pin: %u (Base: 0x%08X)\n", SG2000_LED_GPIO_PIN, SG2000_LED_GPIO_BASE_ADDR);
    printf("Timer Base: 0x%08X\n", SG2000_TIMER_BASE_ADDR);
    printf("Timer IRQ: %u\n", TIMER_IRQ);
    printf("Blink Period: %d ms\n", blink_period_ms);

    /* Setup signal handler */
    if (install_sigint_handler() != 0)
    {
        return 1;
    }

    /* 1. Connect to UMDP */
    conn = umdp_connect();
    if (conn == NULL)
    {
        fprintf(stderr, "Failed to connect to UMDP\n");
        return 1;
    }
    printf("Connected to UMDP successfully\n");

    /* 2. Initialize LED backend */
    ret = led_gpio_init(&led, conn, SG2000_LED_GPIO_PIN);
    if (ret != 0)
    {
        fprintf(stderr, "Failed to initialize LED backend: %s\n", umdp_strerror(ret));
        umdp_disconnect(conn);
        return 1;
    }
    printf("LED backend initialized\n");

    /* 4. Configure Timer IRQ backend */
    ret = timer_irq_init(&timer, conn, blink_period_ms, TIMER_IRQ);
    if (ret != 0)
    {
        fprintf(stderr, "Failed to init timer IRQ backend: %s\n", umdp_strerror(ret));
        led_gpio_cleanup(&led);
        umdp_disconnect(conn);
        return 1;
    }
    printf("Timer interrupt backend initialized\n");

    /* 5. Main loop - wait for timer interrupts and toggle LED */
    printf("\n--- Blinking LED using hardware timer ---\n");
    printf("Press Ctrl+C to stop\n\n");

    while (running)
    {
        ret = timer_irq_wait_tick(&timer);
        if (ret != 0)
        {
            if (ret == ETIMEDOUT)
            {
                fprintf(stderr, "[warn] No timer interrupt received within timeout window\n");
                continue;
            }
            if (running)
            {
                fprintf(stderr, "timer_irq_wait_tick failed: %s\n", umdp_strerror(ret));
            }
            break;
        }

        led_gpio_toggle(&led);
        led_state = led_gpio_read(&led);
        toggle_count++;
        printf("[%u] LED %s\n", toggle_count, led_state ? "ON" : "OFF");
    }

    /* 6. Cleanup */
    printf("\n--- Statistics ---\n");
    printf("Total toggles: %u\n", toggle_count);
    printf("Final LED state: %s\n", led_state ? "ON" : "OFF");

    /* Turn off LED */
    led_gpio_cleanup(&led);

    timer_irq_cleanup(&timer);
    printf("Timer disabled, LED off\n");

    umdp_disconnect(conn);
    printf("UMDP disconnected\n");

    return 0;
}

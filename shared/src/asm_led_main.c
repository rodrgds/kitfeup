/*
 * Assembly-driven LED blinker using UMDP for memory mapping
 *
 * This program uses UMDP to safely map physical GPIO memory into user space,
 * then calls a pure RISC-V assembly function to toggle the onboard LED.
 *
 * Milk-V Duo S onboard LED is on GPIO pin 509 (GPIO1 bank, pin 29)
 * GPIOA base address: 0x03020000
 */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include "umdp.h"

/* Hardware constants from the SG2000 TRM */
#define GPIO_BASE 0x03020000
#define GPIO_SIZE 0x1000
#define LED_PIN 29

/* Tell the C compiler that these functions exist in our assembly file */
extern void led_set_output_asm(volatile uint32_t *gpio_base, uint32_t pin);
extern void led_write_asm(volatile uint32_t *gpio_base, uint32_t pin, uint32_t on);

static volatile sig_atomic_t running = 1;

static void handle_sigint(int sig)
{
    (void)sig;
    running = 0;
}

int main(void)
{
    struct sigaction sa = {0};
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    (void)sigaction(SIGINT, &sa, NULL);

    umdp_connection *conn = umdp_connect();
    if (!conn)
    {
        printf("Failed to connect to UMDP.\n");
        return 1;
    }

    volatile uint32_t *gpio_base = NULL;

    /* Ask Linux/UMDP to map the physical GPIO memory into our safe virtual space */
    if (umdp_mmap_physical(conn, GPIO_BASE, GPIO_SIZE, (void **)&gpio_base) != 0)
    {
        printf("Failed to map memory.\n");
        umdp_disconnect(conn);
        return 1;
    }

    printf("Executing RISC-V Assembly to toggle LED...\n");
    printf("Press Ctrl+C to stop.\n");

    led_set_output_asm(gpio_base, LED_PIN);

    /* Loop forever, writing deterministic LED states from Assembly */
    bool led_on = false;
    while (running)
    {
        led_on = !led_on;
        led_write_asm(gpio_base, LED_PIN, led_on ? 1u : 0u);
        usleep(500000); /* Sleep for 500ms so you can actually see it blink */

        printf("LED is now %s\n", led_on ? "ON" : "OFF");
    }

    led_write_asm(gpio_base, LED_PIN, 0u);

    umdp_disconnect(conn);
    return 0;
}

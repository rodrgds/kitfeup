/*
 * Software PWM Breathing LED Driver
 *
 * GPIOA Base: 0x03020000
 * LED Pin: GPIOA[29] (sysfs #509)
 *
 * NOTE: The onboard LED (GPIOA[29]) does NOT have a PWM alternate function
 * on the SG2000. We must use software PWM by toggling the GPIO very fast
 * with varying duty cycles to create the breathing effect.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <stdbool.h>
#include "../patched-umdp/libumdp/include/umdp.h"

/* GPIO Register Offsets */
#define GPIO_DATA_OFFSET 0x00
#define GPIO_DIR_OFFSET 0x04

/* LED is on GPIOA, pin 29 */
#define GPIO_PIN 29
#define GPIO_BASE_ADDR 0x03020000
#define GPIO_MAP_SIZE 0x1000

/* Software PWM parameters */
#define PWM_PERIOD_US 10000 /* 10ms period (100Hz) */
#define FADE_STEP_US 100    /* Step size for fade */
#define FADE_DELAY_US 5000  /* Delay between brightness changes */

static volatile bool running = true;

static void signal_handler(int sig)
{
    (void)sig;
    running = false;
}

static inline void gpio_set(volatile uint32_t *data_reg, int pin)
{
    *data_reg |= (1u << pin);
}

static inline void gpio_clear(volatile uint32_t *data_reg, int pin)
{
    *data_reg &= ~(1u << pin);
}

int main(void)
{
    umdp_connection *conn;
    volatile uint32_t *gpio_base;
    volatile uint32_t *gpio_data;
    volatile uint32_t *gpio_dir;
    int ret;

    printf("=== Software PWM Breathing LED ===\n");
    printf("GPIO Base: 0x%08X, Pin: %d\n", GPIO_BASE_ADDR, GPIO_PIN);
    printf("PWM Period: %d us, Fade Step: %d us\n", PWM_PERIOD_US, FADE_STEP_US);
    printf("NOTE: Hardware PWM cannot be routed to the onboard LED.\n");
    printf("      Using software PWM via fast GPIO toggling.\n\n");

    signal(SIGINT, signal_handler);

    conn = umdp_connect();
    if (conn == NULL)
    {
        fprintf(stderr, "Failed to connect to UMDP\n");
        return 1;
    }

    ret = umdp_mmap_physical(conn, GPIO_BASE_ADDR, GPIO_MAP_SIZE, (void **)&gpio_base);
    if (ret != 0)
    {
        fprintf(stderr, "Failed to map GPIO: %s\n", umdp_strerror(ret));
        umdp_disconnect(conn);
        return 1;
    }

    gpio_data = gpio_base + (GPIO_DATA_OFFSET / 4);
    gpio_dir = gpio_base + (GPIO_DIR_OFFSET / 4);

    /* Set GPIO pin as output */
    *gpio_dir |= (1u << GPIO_PIN);

    printf("Breathing LED... (Ctrl+C to stop)\n\n");

    int brightness = 0;           /* Current brightness (0 to PWM_PERIOD_US) */
    int direction = FADE_STEP_US; /* Positive = fading in, negative = fading out */

    while (running)
    {
        /* Calculate on and off times based on brightness */
        int on_time = brightness;
        int off_time = PWM_PERIOD_US - brightness;

        /* Software PWM: toggle LED for one PWM cycle */
        if (on_time > 0)
        {
            gpio_set(gpio_data, GPIO_PIN);
            usleep(on_time);
        }
        if (off_time > 0)
        {
            gpio_clear(gpio_data, GPIO_PIN);
            usleep(off_time);
        }

        /* Update brightness */
        brightness += direction;

        /* Reverse direction at limits */
        if (brightness >= PWM_PERIOD_US)
        {
            brightness = PWM_PERIOD_US;
            direction = -FADE_STEP_US;
        }
        else if (brightness <= 0)
        {
            brightness = 0;
            direction = FADE_STEP_US;
        }
    }

    /* Turn off LED */
    gpio_clear(gpio_data, GPIO_PIN);

    printf("\nBreathing LED stopped.\n");
    umdp_disconnect(conn);
    return 0;
}

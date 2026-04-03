/*
 * Step 4: Blink the onboard LED using UMDP with sleep()
 *
 * This program uses umdp_mmap_physical() to directly access GPIO registers
 * instead of using /sys/class/gpio files.
 *
 * Milk-V Duo S onboard LED is on GPIO pin 509 (GPIO1 bank, pin 29)
 * GPIO1 base address: 0x03021000
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "umdp.h"

/* SG2000 GPIO Register Offsets */
#define GPIO_DATA_OFFSET 0x00 /* Data register */
#define GPIO_DIR_OFFSET 0x04  /* Direction register */

/* LED is on GPIOA, pin 29 (GPIO number 509 = 480 + 29) */
#define GPIO_BANK 0
#define GPIO_PIN 29
#define GPIO_BASE_ADDR 0x03020000 /* GPIOA base */
#define GPIO_MAP_SIZE 0x1000

/* Helper to set/clear a bit in a register */
static inline void reg_set_bit(volatile uint32_t *reg, int bit)
{
    *reg |= (1u << bit);
}

static inline void reg_clear_bit(volatile uint32_t *reg, int bit)
{
    *reg &= ~(1u << bit);
}

static inline uint32_t reg_get_bit(volatile uint32_t *reg, int bit)
{
    return (*reg >> bit) & 1u;
}

int main(void)
{
    umdp_connection *conn;
    volatile uint32_t *gpio_base;
    volatile uint32_t *gpio_data;
    volatile uint32_t *gpio_dir;
    int ret;

    printf("=== Step 4: LED Blink using UMDP with sleep() ===\n");
    printf("GPIO Bank: %d, Pin: %d\n", GPIO_BANK, GPIO_PIN);
    printf("Base Address: 0x%08X\n", GPIO_BASE_ADDR);

    /* 1. Connect to UMDP */
    conn = umdp_connect();
    if (conn == NULL)
    {
        fprintf(stderr, "Failed to connect to UMDP\n");
        return 1;
    }
    printf("Connected to UMDP successfully\n");

    /* 2. Map GPIO registers using UMDP */
    ret = umdp_mmap_physical(conn, GPIO_BASE_ADDR, GPIO_MAP_SIZE, (void **)&gpio_base);
    if (ret != 0)
    {
        fprintf(stderr, "Failed to map GPIO registers: %s\n", umdp_strerror(ret));
        umdp_disconnect(conn);
        return 1;
    }
    printf("GPIO registers mapped at virtual address: %p\n", (void *)gpio_base);

    /* Calculate register addresses */
    gpio_data = gpio_base + (GPIO_DATA_OFFSET / 4);
    gpio_dir = gpio_base + (GPIO_DIR_OFFSET / 4);

    /* 3. Set GPIO pin as output */
    reg_set_bit(gpio_dir, GPIO_PIN);
    printf("GPIO pin %d set as OUTPUT\n", GPIO_PIN);

    /* 4. Blink loop */
    printf("Blinking LED... Press Ctrl+C to stop.\n");
    printf("Using sleep() for timing (500ms on, 500ms off)\n");

    int count = 0;
    while (1)
    {
        /* Turn LED ON (set pin high) */
        reg_set_bit(gpio_data, GPIO_PIN);
        printf("[%d] LED ON\n", count++);
        usleep(500000); /* 500ms */

        /* Turn LED OFF (clear pin low) */
        reg_clear_bit(gpio_data, GPIO_PIN);
        printf("[%d] LED OFF\n", count++);
        usleep(500000); /* 500ms */
    }

    /* Cleanup (never reached in this example) */
    umdp_disconnect(conn);
    return 0;
}

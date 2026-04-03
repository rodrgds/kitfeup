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
#include "umdp.h"

/* Hardware constants from the SG2000 TRM */
#define GPIO_BASE 0x03020000
#define GPIO_SIZE 0x1000
#define LED_PIN 29

/* Tell the C compiler that this function exists in our assembly file */
extern void toggle_led_asm(volatile uint32_t *gpio_base, uint32_t pin);

int main(void)
{
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

    /* Loop forever, calling our custom Assembly function */
    while (1)
    {
        toggle_led_asm(gpio_base, LED_PIN);
        usleep(500000); /* Sleep for 500ms so you can actually see it blink */
    }

    umdp_disconnect(conn);
    return 0;
}

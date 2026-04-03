/*
 * Watchdog Timer Driver - Hardware failsafe that reboots if not pet
 *
 * Base Address: 0x03010000 (WDT0)
 *
 * Timeout formula: 2^(16 + range) / 25MHz
 * range 0x0B = 2^27 / 25M = ~5.36 seconds
 *
 * WARNING: This program WILL reboot your board if you stop petting the dog!
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "umdp.h"

/* WDT Register Offsets */
#define WDT_CR 0x00   /* Control Register - Bit 0 enables */
#define WDT_TORR 0x04 /* Timeout Range */
#define WDT_CRR 0x0C  /* Counter Restart (pet the dog here) */

/* WDT Base Address */
#define WDT_BASE_ADDR 0x03010000
#define WDT_MAP_SIZE 0x1000

/* Magic password to restart the watchdog counter */
#define WDT_FEED_PASSWORD 0x76

/*
 * Timeout range: 0x0B (11)
 * 2^(16+11) = 2^27 = 134,217,728 cycles
 * At 25MHz: 134,217,728 / 25,000,000 = ~5.36 seconds
 */
#define WDT_TIMEOUT_RANGE 0x0B

static inline void wdt_write(volatile uint32_t *base, uint32_t offset, uint32_t value)
{
    *(base + (offset / 4)) = value;
}

static inline uint32_t wdt_read(volatile uint32_t *base, uint32_t offset)
{
    return *(base + (offset / 4));
}

int main(int argc, char *argv[])
{
    umdp_connection *conn;
    volatile uint32_t *wdt_base;
    int ret;
    int max_loops = 10;
    int enable_reboot_test = 0;

    if (argc > 1)
    {
        max_loops = atoi(argv[1]);
    }
    if (argc > 2 && argv[2][0] == 'y')
    {
        enable_reboot_test = 1;
    }

    printf("=== Watchdog Timer Driver ===\n");
    printf("WDT Base: 0x%08X\n", WDT_BASE_ADDR);
    printf("Timeout Range: 0x%02X (~5.36 seconds)\n", WDT_TIMEOUT_RANGE);
    printf("Feed Password: 0x%02X\n", WDT_FEED_PASSWORD);
    printf("Usage: %s [max_loops] [y=reboot_test]\n\n", argv[0]);

    conn = umdp_connect();
    if (conn == NULL)
    {
        fprintf(stderr, "Failed to connect to UMDP\n");
        return 1;
    }

    ret = umdp_mmap_physical(conn, WDT_BASE_ADDR, WDT_MAP_SIZE, (void **)&wdt_base);
    if (ret != 0)
    {
        fprintf(stderr, "Failed to map WDT: %s\n", umdp_strerror(ret));
        umdp_disconnect(conn);
        return 1;
    }

    /* Set timeout range */
    wdt_write(wdt_base, WDT_TORR, WDT_TIMEOUT_RANGE);

    /* Enable the watchdog */
    uint32_t ctrl = wdt_read(wdt_base, WDT_CR);
    ctrl |= 1;
    wdt_write(wdt_base, WDT_CR, ctrl);
    printf("WDT ENABLED! Timeout ~5.36 seconds\n\n");

    int loops = 0;
    while (loops < max_loops)
    {
        wdt_write(wdt_base, WDT_CRR, WDT_FEED_PASSWORD);
        printf("[%d] Dog pet!\n", loops);
        sleep(1);
        loops++;
    }

    if (enable_reboot_test)
    {
        printf("\n*** STOPPING PETTING! Board will reboot in ~5 seconds... ***\n");
        while (1)
        {
            /* Do nothing - watchdog will trigger reboot */
        }
    }
    else
    {
        /* Disable watchdog */
        ctrl = wdt_read(wdt_base, WDT_CR);
        ctrl &= ~1;
        wdt_write(wdt_base, WDT_CR, ctrl);
        printf("\nWatchdog disabled. Safe exit.\n");
    }

    umdp_disconnect(conn);
    return 0;
}

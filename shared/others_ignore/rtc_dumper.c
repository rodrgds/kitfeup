/*
 * RTC Register Dumper - Find the ticking seconds counter
 *
 * Dumps the first 8 registers of the RTC block in a loop.
 * Watch for the column that increases by 1 every second - that's your RTC_SEC_OFFSET!
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "umdp.h"

#define RTC_BASE_ADDR 0x05026000
#define RTC_MAP_SIZE 0x1000

static inline uint32_t rtc_read(volatile uint32_t *base, uint32_t offset)
{
    return *(base + (offset / 4));
}

int main(void)
{
    umdp_connection *conn = umdp_connect();
    if (!conn)
    {
        fprintf(stderr, "Failed to connect to UMDP\n");
        return 1;
    }

    volatile uint32_t *rtc_base = NULL;
    if (umdp_mmap_physical(conn, RTC_BASE_ADDR, RTC_MAP_SIZE, (void **)&rtc_base) != 0)
    {
        fprintf(stderr, "Failed to map RTC registers\n");
        umdp_disconnect(conn);
        return 1;
    }

    printf("=== RTC Register Dumper ===\n");
    printf("Finding the ticking seconds counter...\n\n");
    printf("Offset: |   0x00   |   0x04   |   0x08   |   0x0C   |   0x10   |   0x14   |   0x18   |   0x1C   |\n");
    printf("--------+----------+----------+----------+----------+----------+----------+----------+----------+\n");

    for (int i = 0; i < 5; i++)
    {
        printf("Read %d: |", i);
        for (uint32_t offset = 0; offset <= 0x1C; offset += 4)
        {
            printf(" %08X |", rtc_read(rtc_base, offset));
        }
        printf("\n");
        sleep(1);
    }

    umdp_disconnect(conn);
    return 0;
}

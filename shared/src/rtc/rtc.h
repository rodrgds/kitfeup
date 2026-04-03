#ifndef RTC_H
#define RTC_H

#include <stdbool.h>
#include <stdint.h>

#include "umdp.h"

#define SG2000_RTC_BASE_ADDR 0x05026000u
#define SG2000_RTC_MAP_SIZE 0x1000u

typedef struct {
    umdp_connection* conn;
    volatile uint32_t* rtc_base;
    bool ready;
} rtc_ctx;

int rtc_init(rtc_ctx* ctx, umdp_connection* conn);
int rtc_ensure_running(rtc_ctx* ctx, uint32_t fallback_epoch);
uint32_t rtc_read_seconds(rtc_ctx* ctx);

#endif

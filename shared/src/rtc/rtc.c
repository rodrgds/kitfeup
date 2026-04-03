#include "rtc.h"

#include <stddef.h>

#define RTC_ANA_CALIB_OFFSET 0x000u
#define RTC_SEC_PULSE_GEN_OFFSET 0x004u
#define RTC_ALARM_ENABLE_OFFSET 0x00Cu
#define RTC_SET_SEC_CNTR_VALUE_OFFSET 0x010u
#define RTC_SET_SEC_CNTR_TRIG_OFFSET 0x014u
#define RTC_SEC_CNTR_VALUE_OFFSET 0x018u

#define RTC_MACRO_RG_SET_T_OFFSET 0x498u
#define RTC_MACRO_DA_CLEAR_ALL_OFFSET 0x480u
#define RTC_MACRO_DA_SOC_READY_OFFSET 0x48Cu

#define RTC_ENABLE_BIT (1u << 31)

static inline uint32_t rtc_reg_read(volatile uint32_t* base, uint32_t offset) {
    return *(base + (offset / 4u));
}

static inline void rtc_reg_write(volatile uint32_t* base, uint32_t offset, uint32_t value) {
    *(base + (offset / 4u)) = value;
}

int rtc_init(rtc_ctx* ctx, umdp_connection* conn) {
    if (ctx == NULL || conn == NULL) {
        return -1;
    }

    ctx->conn = conn;
    ctx->rtc_base = NULL;
    ctx->ready = false;

    int ret = umdp_mmap_physical(ctx->conn, SG2000_RTC_BASE_ADDR, SG2000_RTC_MAP_SIZE, (void**) &ctx->rtc_base);
    if (ret != 0) {
        return ret;
    }

    ctx->ready = true;
    return 0;
}

int rtc_ensure_running(rtc_ctx* ctx, uint32_t fallback_epoch) {
    if (ctx == NULL || !ctx->ready) {
        return -1;
    }

    uint32_t value = rtc_reg_read(ctx->rtc_base, RTC_SEC_PULSE_GEN_OFFSET) & ~RTC_ENABLE_BIT;
    rtc_reg_write(ctx->rtc_base, RTC_SEC_PULSE_GEN_OFFSET, value);
    value = rtc_reg_read(ctx->rtc_base, RTC_ANA_CALIB_OFFSET) & ~RTC_ENABLE_BIT;
    rtc_reg_write(ctx->rtc_base, RTC_ANA_CALIB_OFFSET, value);
    rtc_reg_write(ctx->rtc_base, RTC_ALARM_ENABLE_OFFSET, 0u);

    uint32_t rtc_seconds = rtc_reg_read(ctx->rtc_base, RTC_SEC_CNTR_VALUE_OFFSET);
    if (rtc_seconds < 0x30000000u && fallback_epoch != 0u) {
        rtc_reg_write(ctx->rtc_base, RTC_SET_SEC_CNTR_VALUE_OFFSET, fallback_epoch);
        rtc_reg_write(ctx->rtc_base, RTC_SET_SEC_CNTR_TRIG_OFFSET, 1u);
        rtc_reg_write(ctx->rtc_base, RTC_MACRO_RG_SET_T_OFFSET, fallback_epoch);

        rtc_reg_write(ctx->rtc_base, RTC_MACRO_DA_CLEAR_ALL_OFFSET, 1u);
        rtc_reg_write(ctx->rtc_base, RTC_MACRO_DA_SOC_READY_OFFSET, 1u);
        rtc_reg_write(ctx->rtc_base, RTC_MACRO_DA_CLEAR_ALL_OFFSET, 0u);
        rtc_reg_write(ctx->rtc_base, RTC_MACRO_RG_SET_T_OFFSET, 0u);
        rtc_reg_write(ctx->rtc_base, RTC_MACRO_DA_SOC_READY_OFFSET, 0u);
    }

    return 0;
}

uint32_t rtc_read_seconds(rtc_ctx* ctx) {
    if (ctx == NULL || !ctx->ready) {
        return 0u;
    }
    return rtc_reg_read(ctx->rtc_base, RTC_SEC_CNTR_VALUE_OFFSET);
}

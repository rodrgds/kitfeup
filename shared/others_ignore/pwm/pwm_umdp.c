#include "pwm_umdp.h"

#include <stddef.h>

#define REG_HLPERIOD 0x00u
#define REG_PERIOD 0x04u
#define REG_GROUP_STRIDE 0x08u

#define REG_PWMSTART 0x44u
#define REG_PWMUPDATE 0x4Cu
#define REG_PWM_OE 0xD0u

static inline uint32_t pwm_read(volatile uint32_t* base, uint32_t offset) {
    return *(base + (offset / 4u));
}

static inline void pwm_write(volatile uint32_t* base, uint32_t offset, uint32_t value) {
    *(base + (offset / 4u)) = value;
}

static inline uint32_t pwm_channel_offset(uint32_t channel, uint32_t reg_offset) {
    return (channel * REG_GROUP_STRIDE) + reg_offset;
}

static inline uint32_t bit_mask(uint32_t channel) {
    return (1u << channel);
}

int pwm_umdp_init(pwm_umdp_ctx* ctx, umdp_connection* conn, uint32_t base_addr, uint32_t channel, uint32_t clk_hz) {
    if (ctx == NULL || conn == NULL || clk_hz == 0u || channel >= SG2000_PWM_CHANNELS_PER_CONTROLLER) {
        return -1;
    }

    ctx->conn = conn;
    ctx->pwm_base = NULL;
    ctx->base_addr = base_addr;
    ctx->channel = channel;
    ctx->clk_hz = clk_hz;
    ctx->period_cycles = 0u;
    ctx->duty_cycles = 0u;
    ctx->ready = false;
    ctx->enabled = false;

    int ret = umdp_mmap_physical(ctx->conn, ctx->base_addr, SG2000_PWM_MAP_SIZE, (void**) &ctx->pwm_base);
    if (ret != 0) {
        return ret;
    }

    ctx->ready = true;
    return 0;
}

int pwm_umdp_config_cycles(pwm_umdp_ctx* ctx, uint32_t period_cycles, uint32_t duty_cycles) {
    if (ctx == NULL || !ctx->ready || period_cycles < 2u) {
        return -1;
    }

    if (duty_cycles == 0u) {
        duty_cycles = 1u;
    }
    if (duty_cycles >= period_cycles) {
        duty_cycles = period_cycles - 1u;
    }

    uint32_t hlperiod = period_cycles - duty_cycles;

    pwm_write(ctx->pwm_base, pwm_channel_offset(ctx->channel, REG_PERIOD), period_cycles);
    pwm_write(ctx->pwm_base, pwm_channel_offset(ctx->channel, REG_HLPERIOD), hlperiod);

    uint32_t value = pwm_read(ctx->pwm_base, REG_PWMSTART);
    value |= bit_mask(ctx->channel);
    pwm_write(ctx->pwm_base, REG_PWMUPDATE, value);
    value &= ~bit_mask(ctx->channel);
    pwm_write(ctx->pwm_base, REG_PWMUPDATE, value);

    ctx->period_cycles = period_cycles;
    ctx->duty_cycles = duty_cycles;
    return 0;
}

int pwm_umdp_config_ns(pwm_umdp_ctx* ctx, uint32_t period_ns, uint32_t duty_ns) {
    if (ctx == NULL || !ctx->ready || period_ns == 0u || duty_ns > period_ns) {
        return -1;
    }

    uint64_t period_cycles_64 = (uint64_t)ctx->clk_hz * (uint64_t)period_ns;
    period_cycles_64 /= 1000000000ull;
    if (period_cycles_64 < 2ull) {
        period_cycles_64 = 2ull;
    }
    if (period_cycles_64 > 0xFFFFFFFFull) {
        period_cycles_64 = 0xFFFFFFFFull;
    }

    uint64_t duty_cycles_64 = period_cycles_64 * (uint64_t)duty_ns;
    duty_cycles_64 /= (uint64_t)period_ns;
    if (duty_cycles_64 > 0xFFFFFFFFull) {
        duty_cycles_64 = 0xFFFFFFFFull;
    }

    return pwm_umdp_config_cycles(ctx, (uint32_t)period_cycles_64, (uint32_t)duty_cycles_64);
}

int pwm_umdp_enable(pwm_umdp_ctx* ctx) {
    if (ctx == NULL || !ctx->ready) {
        return -1;
    }

    uint32_t start_value = pwm_read(ctx->pwm_base, REG_PWMSTART);
    start_value &= ~bit_mask(ctx->channel);
    pwm_write(ctx->pwm_base, REG_PWMSTART, start_value);

    uint32_t value = start_value | bit_mask(ctx->channel);
    pwm_write(ctx->pwm_base, REG_PWM_OE, value);
    pwm_write(ctx->pwm_base, REG_PWMSTART, value);

    ctx->enabled = true;
    return 0;
}

void pwm_umdp_disable(pwm_umdp_ctx* ctx) {
    if (ctx == NULL || !ctx->ready) {
        return;
    }

    uint32_t value = pwm_read(ctx->pwm_base, REG_PWMSTART) & ~bit_mask(ctx->channel);
    pwm_write(ctx->pwm_base, REG_PWM_OE, value);
    pwm_write(ctx->pwm_base, REG_PWMSTART, value);

    pwm_write(ctx->pwm_base, pwm_channel_offset(ctx->channel, REG_PERIOD), 1u);
    pwm_write(ctx->pwm_base, pwm_channel_offset(ctx->channel, REG_HLPERIOD), 2u);

    ctx->enabled = false;
}

void pwm_umdp_cleanup(pwm_umdp_ctx* ctx) {
    if (ctx == NULL) {
        return;
    }

    pwm_umdp_disable(ctx);
    ctx->ready = false;
}

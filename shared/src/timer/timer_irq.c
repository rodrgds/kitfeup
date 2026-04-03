#include "timer_irq.h"

#include <errno.h>
#include <stddef.h>
#include <time.h>

#define TIMER_LOAD_OFFSET 0x00u
#define TIMER_CURRENT_VALUE_OFFSET 0x04u
#define TIMER_CONTROL_OFFSET 0x08u
#define TIMER_EOI_OFFSET 0x0Cu
#define TIMER_INT_STATUS_OFFSET 0x10u

#define TIMER_ENABLE (1u << 0)
#define TIMER_MODE_PERIODIC (1u << 1)

static inline void timer_write(volatile uint32_t* base, uint32_t offset, uint32_t value) {
    base[offset / sizeof(uint32_t)] = value;
}

static inline uint32_t timer_read(volatile uint32_t* base, uint32_t offset) {
    return base[offset / sizeof(uint32_t)];
}

static uint64_t monotonic_ms(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0u;
    }
    return (uint64_t) ts.tv_sec * 1000u + (uint64_t) (ts.tv_nsec / 1000000u);
}

int timer_irq_init(timer_irq_ctx* ctx, umdp_connection* conn, uint32_t period_ms, uint32_t irq) {
    if (ctx == NULL || conn == NULL || period_ms == 0u) {
        return EINVAL;
    }

    ctx->conn = conn;
    ctx->timer_base = NULL;
    ctx->irq = irq;
    ctx->period_ms = period_ms;
    ctx->load_value = 0u;
    ctx->last_tick_ms = 0u;
    ctx->subscribed = false;
    ctx->started = false;

    {
        void* mapped = NULL;
        int ret = umdp_mmap_physical(ctx->conn, (off_t) SG2000_TIMER_BASE_ADDR, SG2000_TIMER_MAP_SIZE, &mapped);
        if (ret != 0) {
            return ret;
        }
        ctx->timer_base = (volatile uint32_t*) mapped;
    }

    ctx->load_value = (SG2000_TIMER_FREQ_HZ / 1000u) * period_ms;

    timer_write(ctx->timer_base, TIMER_CONTROL_OFFSET, 0u);
    (void) timer_read(ctx->timer_base, TIMER_EOI_OFFSET);
    timer_write(ctx->timer_base, TIMER_LOAD_OFFSET, ctx->load_value);

    {
        int ret = umdp_interrupt_subscribe(ctx->conn, ctx->irq);
        if (ret != 0) {
            return ret;
        }
    }

    ctx->subscribed = true;
    timer_write(ctx->timer_base, TIMER_CONTROL_OFFSET, TIMER_ENABLE | TIMER_MODE_PERIODIC);
    ctx->started = true;

    return 0;
}

int timer_irq_wait_tick(timer_irq_ctx* ctx) {
    uint32_t received_irq = 0u;

    if (ctx == NULL || !ctx->subscribed || ctx->timer_base == NULL) {
        return EINVAL;
    }

    while (true) {
        int ret = umdp_receive_interrupt_timeout(ctx->conn, &received_irq, ctx->period_ms * 4u);
        if (ret == ETIMEDOUT) {
            /* No IRQ arrived; clear any latched status and ensure IRQ line is unmasked. */
            (void) timer_read(ctx->timer_base, TIMER_INT_STATUS_OFFSET);
            (void) timer_read(ctx->timer_base, TIMER_EOI_OFFSET);
            (void) umdp_interrupt_unmask(ctx->conn, ctx->irq);
            return ETIMEDOUT;
        }
        if (ret != 0) {
            return ret;
        }

        if (received_irq == ctx->irq) {
            (void) timer_read(ctx->timer_base, TIMER_INT_STATUS_OFFSET);
            (void) timer_read(ctx->timer_base, TIMER_EOI_OFFSET);
            (void) umdp_interrupt_unmask(ctx->conn, ctx->irq);

            {
                uint64_t now_ms = monotonic_ms();
                if (ctx->last_tick_ms != 0u && now_ms != 0u) {
                    uint64_t elapsed_ms = now_ms - ctx->last_tick_ms;
                    if (elapsed_ms < ((uint64_t) ctx->period_ms * 4u) / 5u) {
                        continue;
                    }
                }
                if (now_ms != 0u) {
                    ctx->last_tick_ms = now_ms;
                }
            }

            return 0;
        }
    }
}

void timer_irq_stop(timer_irq_ctx* ctx) {
    if (ctx == NULL) {
        return;
    }

    if (ctx->started && ctx->timer_base != NULL) {
        timer_write(ctx->timer_base, TIMER_CONTROL_OFFSET, 0u);
        (void) timer_read(ctx->timer_base, TIMER_EOI_OFFSET);
        (void) timer_read(ctx->timer_base, TIMER_CURRENT_VALUE_OFFSET);
        ctx->started = false;
    }

    if (ctx->subscribed) {
        (void) umdp_interrupt_unsubscribe(ctx->conn, ctx->irq);
        ctx->subscribed = false;
    }
}

void timer_irq_cleanup(timer_irq_ctx* ctx) {
    if (ctx == NULL) {
        return;
    }

    timer_irq_stop(ctx);
}

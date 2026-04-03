#include "timer_multi.h"

#include <errno.h>

int timer_multi_init(timer_multi_ctx* ctx, umdp_connection* conn, uint32_t base_tick_ms, uint32_t irq) {
    if (ctx == NULL || conn == NULL || base_tick_ms == 0u) {
        return EINVAL;
    }

    ctx->base_tick_ms = base_tick_ms;
    ctx->elapsed_ms = 0u;
    ctx->timer_count = 0u;

    for (size_t i = 0; i < TIMER_MULTI_MAX_TIMERS; i++) {
        ctx->timers[i].id = 0u;
        ctx->timers[i].period_ms = 0u;
        ctx->timers[i].next_due_ms = 0u;
        ctx->timers[i].active = false;
    }

    return timer_irq_init(&ctx->irq, conn, base_tick_ms, irq);
}

int timer_multi_add(timer_multi_ctx* ctx, uint32_t id, uint32_t period_ms) {
    if (ctx == NULL || period_ms == 0u) {
        return EINVAL;
    }

    if (ctx->timer_count >= TIMER_MULTI_MAX_TIMERS) {
        return ENOSPC;
    }

    for (size_t i = 0; i < ctx->timer_count; i++) {
        if (ctx->timers[i].active && ctx->timers[i].id == id) {
            return EEXIST;
        }
    }

    timer_multi_entry* entry = &ctx->timers[ctx->timer_count];
    entry->id = id;
    entry->period_ms = period_ms;
    entry->next_due_ms = ctx->elapsed_ms + (uint64_t) period_ms;
    entry->active = true;
    ctx->timer_count++;

    return 0;
}

int timer_multi_wait(timer_multi_ctx* ctx, uint32_t* fired_ids, size_t fired_ids_capacity, size_t* fired_count) {
    if (ctx == NULL || fired_ids == NULL || fired_count == NULL || fired_ids_capacity == 0u) {
        return EINVAL;
    }

    *fired_count = 0u;

    while (*fired_count == 0u) {
        int ret = timer_irq_wait_tick(&ctx->irq);
        if (ret != 0) {
            return ret;
        }

        ctx->elapsed_ms += (uint64_t) ctx->base_tick_ms;

        for (size_t i = 0; i < ctx->timer_count; i++) {
            timer_multi_entry* entry = &ctx->timers[i];
            if (!entry->active) {
                continue;
            }

            if (ctx->elapsed_ms >= entry->next_due_ms) {
                if (*fired_count < fired_ids_capacity) {
                    fired_ids[*fired_count] = entry->id;
                    (*fired_count)++;
                }

                while (entry->next_due_ms <= ctx->elapsed_ms) {
                    entry->next_due_ms += (uint64_t) entry->period_ms;
                }
            }
        }
    }

    return 0;
}

void timer_multi_cleanup(timer_multi_ctx* ctx) {
    if (ctx == NULL) {
        return;
    }

    timer_irq_cleanup(&ctx->irq);
}

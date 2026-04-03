#ifndef TIMER_MULTI_H
#define TIMER_MULTI_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "timer_irq.h"

#define TIMER_MULTI_MAX_TIMERS 16u

typedef struct {
    uint32_t id;
    uint32_t period_ms;
    uint64_t next_due_ms;
    bool active;
} timer_multi_entry;

typedef struct {
    timer_irq_ctx irq;
    uint32_t base_tick_ms;
    uint64_t elapsed_ms;
    timer_multi_entry timers[TIMER_MULTI_MAX_TIMERS];
    size_t timer_count;
} timer_multi_ctx;

int timer_multi_init(timer_multi_ctx* ctx, umdp_connection* conn, uint32_t base_tick_ms, uint32_t irq);
int timer_multi_add(timer_multi_ctx* ctx, uint32_t id, uint32_t period_ms);
int timer_multi_wait(timer_multi_ctx* ctx, uint32_t* fired_ids, size_t fired_ids_capacity, size_t* fired_count);
void timer_multi_cleanup(timer_multi_ctx* ctx);

#endif

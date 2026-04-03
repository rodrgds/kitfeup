#ifndef TIMER_IRQ_H
#define TIMER_IRQ_H

#include <stdbool.h>
#include <stdint.h>

#include "umdp.h"

#define SG2000_TIMER_BASE_ADDR 0x030A0000u
#define SG2000_TIMER_MAP_SIZE 0x1000u
#define SG2000_TIMER_FREQ_HZ 25000000u
#define SG2000_TIMER_DEFAULT_IRQ 79u

typedef struct {
    umdp_connection* conn;
    volatile uint32_t* timer_base;
    uint32_t irq;
    uint32_t period_ms;
    uint32_t load_value;
    uint64_t last_tick_ms;
    bool subscribed;
    bool started;
} timer_irq_ctx;

int timer_irq_init(timer_irq_ctx* ctx, umdp_connection* conn, uint32_t period_ms, uint32_t irq);
int timer_irq_wait_tick(timer_irq_ctx* ctx);
void timer_irq_stop(timer_irq_ctx* ctx);
void timer_irq_cleanup(timer_irq_ctx* ctx);

#endif

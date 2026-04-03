#ifndef PWM_UMDP_H
#define PWM_UMDP_H

#include <stdbool.h>
#include <stdint.h>

#include "umdp.h"

#define SG2000_PWM_MAP_SIZE 0x1000u

#define SG2000_PWM0_BASE_ADDR 0x03060000u
#define SG2000_PWM1_BASE_ADDR 0x03061000u
#define SG2000_PWM2_BASE_ADDR 0x03062000u
#define SG2000_PWM3_BASE_ADDR 0x03063000u

#define SG2000_PWM_CHANNELS_PER_CONTROLLER 4u

typedef struct {
    umdp_connection* conn;
    volatile uint32_t* pwm_base;
    uint32_t base_addr;
    uint32_t channel;
    uint32_t clk_hz;
    uint32_t period_cycles;
    uint32_t duty_cycles;
    bool ready;
    bool enabled;
} pwm_umdp_ctx;

int pwm_umdp_init(pwm_umdp_ctx* ctx, umdp_connection* conn, uint32_t base_addr, uint32_t channel, uint32_t clk_hz);
int pwm_umdp_config_cycles(pwm_umdp_ctx* ctx, uint32_t period_cycles, uint32_t duty_cycles);
int pwm_umdp_config_ns(pwm_umdp_ctx* ctx, uint32_t period_ns, uint32_t duty_ns);
int pwm_umdp_enable(pwm_umdp_ctx* ctx);
void pwm_umdp_disable(pwm_umdp_ctx* ctx);
void pwm_umdp_cleanup(pwm_umdp_ctx* ctx);

#endif

#include "led_gpio.h"

#include <stddef.h>

#define GPIO_DATA_OFFSET 0x00u
#define GPIO_DIR_OFFSET 0x04u

static inline volatile uint32_t* gpio_data_reg(volatile uint32_t* base) {
    return base + (GPIO_DATA_OFFSET / 4u);
}

static inline volatile uint32_t* gpio_dir_reg(volatile uint32_t* base) {
    return base + (GPIO_DIR_OFFSET / 4u);
}

int led_gpio_init(led_gpio_ctx* ctx, umdp_connection* conn, uint32_t pin) {
    if (ctx == NULL || conn == NULL) {
        return -1;
    }

    ctx->conn = conn;
    ctx->gpio_base = NULL;
    ctx->pin = pin;
    ctx->ready = false;

    int ret = umdp_mmap_physical(ctx->conn, SG2000_LED_GPIO_BASE_ADDR, SG2000_LED_GPIO_MAP_SIZE, (void**) &ctx->gpio_base);
    if (ret != 0) {
        return ret;
    }

    *gpio_dir_reg(ctx->gpio_base) |= (1u << ctx->pin);
    *gpio_data_reg(ctx->gpio_base) &= ~(1u << ctx->pin);
    ctx->ready = true;
    return 0;
}

void led_gpio_write(led_gpio_ctx* ctx, bool on) {
    if (ctx == NULL || !ctx->ready) {
        return;
    }

    if (on) {
        *gpio_data_reg(ctx->gpio_base) |= (1u << ctx->pin);
    } else {
        *gpio_data_reg(ctx->gpio_base) &= ~(1u << ctx->pin);
    }
}

void led_gpio_toggle(led_gpio_ctx* ctx) {
    if (ctx == NULL || !ctx->ready) {
        return;
    }
    *gpio_data_reg(ctx->gpio_base) ^= (1u << ctx->pin);
}

bool led_gpio_read(led_gpio_ctx* ctx) {
    if (ctx == NULL || !ctx->ready) {
        return false;
    }
    return (((*gpio_data_reg(ctx->gpio_base)) >> ctx->pin) & 1u) != 0u;
}

void led_gpio_cleanup(led_gpio_ctx* ctx) {
    if (ctx == NULL || !ctx->ready) {
        return;
    }
    led_gpio_write(ctx, false);
    ctx->ready = false;
}

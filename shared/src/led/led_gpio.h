#ifndef LED_GPIO_H
#define LED_GPIO_H

#include <stdbool.h>
#include <stdint.h>

#include "umdp.h"

#define SG2000_LED_GPIO_BASE_ADDR 0x03020000u
#define SG2000_LED_GPIO_MAP_SIZE 0x1000u
#define SG2000_LED_GPIO_PIN 29u

typedef struct {
    umdp_connection* conn;
    volatile uint32_t* gpio_base;
    uint32_t pin;
    bool ready;
} led_gpio_ctx;

int led_gpio_init(led_gpio_ctx* ctx, umdp_connection* conn, uint32_t pin);
void led_gpio_write(led_gpio_ctx* ctx, bool on);
void led_gpio_toggle(led_gpio_ctx* ctx);
bool led_gpio_read(led_gpio_ctx* ctx);
void led_gpio_cleanup(led_gpio_ctx* ctx);

#endif

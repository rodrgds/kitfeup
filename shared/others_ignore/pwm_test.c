/*
 * UMDP PWM test program for SG2000
 *
 * Usage:
 *   pwm_test [controller] [channel] [period_ns] [duty_percent]
 *
 * Defaults:
 *   controller=0, channel=0, period_ns=1000000 (1 kHz), duty_percent=50
 */

#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "umdp.h"
#include "pwm/pwm_umdp.h"

static volatile bool running = true;

static void signal_handler(int sig) {
    (void)sig;
    running = false;
    printf("\nStopping PWM...\n");
}

static int install_sigint_handler(void) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) != 0) {
        perror("sigaction");
        return 1;
    }
    return 0;
}

static uint32_t controller_to_base(uint32_t controller) {
    switch (controller) {
        case 0: return SG2000_PWM0_BASE_ADDR;
        case 1: return SG2000_PWM1_BASE_ADDR;
        case 2: return SG2000_PWM2_BASE_ADDR;
        case 3: return SG2000_PWM3_BASE_ADDR;
        default: return SG2000_PWM0_BASE_ADDR;
    }
}

int main(int argc, char* argv[]) {
    uint32_t controller = 0u;
    uint32_t channel = 0u;
    uint32_t period_ns = 1000000u;
    uint32_t duty_percent = 50u;

    if (argc > 1) {
        controller = (uint32_t)atoi(argv[1]);
    }
    if (argc > 2) {
        channel = (uint32_t)atoi(argv[2]);
    }
    if (argc > 3) {
        period_ns = (uint32_t)atoi(argv[3]);
    }
    if (argc > 4) {
        duty_percent = (uint32_t)atoi(argv[4]);
    }

    if (controller > 3u || channel >= SG2000_PWM_CHANNELS_PER_CONTROLLER || period_ns == 0u || duty_percent > 100u) {
        fprintf(stderr,
                "Usage: %s [controller:0-3] [channel:0-3] [period_ns] [duty_percent:0-100]\n",
                argv[0]);
        return 1;
    }

    uint32_t base_addr = controller_to_base(controller);
    uint32_t duty_ns = (uint32_t)(((uint64_t)period_ns * duty_percent) / 100ull);

    printf("=== UMDP PWM Test ===\n");
    printf("Controller: %u (base 0x%08X)\n", controller, base_addr);
    printf("Channel: %u\n", channel);
    printf("Period: %u ns\n", period_ns);
    printf("Duty: %u%% (%u ns)\n", duty_percent, duty_ns);
    printf("Note: make sure pinmux routes this PWM channel to your target pin.\n");

    if (install_sigint_handler() != 0) {
        return 1;
    }

    umdp_connection* conn = umdp_connect();
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to UMDP\n");
        return 1;
    }

    pwm_umdp_ctx pwm;
    int ret = pwm_umdp_init(&pwm, conn, base_addr, channel, 100000000u);
    if (ret != 0) {
        fprintf(stderr, "pwm_umdp_init failed: %s\n", umdp_strerror(ret));
        umdp_disconnect(conn);
        return 1;
    }

    ret = pwm_umdp_config_ns(&pwm, period_ns, duty_ns);
    if (ret != 0) {
        fprintf(stderr, "pwm_umdp_config_ns failed: %d\n", ret);
        pwm_umdp_cleanup(&pwm);
        umdp_disconnect(conn);
        return 1;
    }

    ret = pwm_umdp_enable(&pwm);
    if (ret != 0) {
        fprintf(stderr, "pwm_umdp_enable failed: %d\n", ret);
        pwm_umdp_cleanup(&pwm);
        umdp_disconnect(conn);
        return 1;
    }

    printf("PWM enabled. Press Ctrl+C to stop.\n");
    while (running) {
        sleep(1);
    }

    pwm_umdp_cleanup(&pwm);
    umdp_disconnect(conn);
    printf("PWM disabled and cleaned up.\n");
    return 0;
}

#include "board_commands.h"

#include "common.h"

#include <stdio.h>
#include <unistd.h>

static bool timer_irq_functional_probe(void) {
    int rc;

    if (!path_exists("/root/shared/compiled/timer_probe")) {
        return false;
    }

    rc = run_cmd("/root/shared/compiled/timer_probe >/tmp/kitfeup_timer_probe.log 2>&1");
    if (rc != 0) {
        return false;
    }
    return file_contains("/tmp/kitfeup_timer_probe.log", "TIMER_PROBE_OK");
}

void board_print_help(void) {
    puts("kitfeup (board mode)");
    puts("");
    puts("Usage:");
    puts("  kitfeup              Same as 'kitfeup doctor'");
    puts("  kitfeup doctor       Board readiness report");
    puts("  kitfeup setup        First-boot board setup (resize + disable blink)");
    puts("  kitfeup run <prog> [args...]  Run /root/shared/compiled/<prog>");
    puts("");
    puts("Board plugins:");
    puts("  kitfeup <plugin> ...");
    puts("  Search path: kitfeup.d/board/<plugin>, ~/.config/kitfeup/plugins/board/<plugin>");
}

int board_doctor(void) {
    int ok = 0;
    int miss = 0;

    printf("[kitfeup-cli] Board doctor report\n");

    if (path_exists("/dev/umdp-mem")) {
        printf("  [OK]   /dev/umdp-mem exists (umdp module loaded)\n");
        ok++;
    } else {
        printf("  [MISS] /dev/umdp-mem exists (umdp module loaded)\n");
        printf("         Fix: insmod /root/shared/umdp.ko\n");
        miss++;
    }

    if (path_exists("/proc/umdp/permtab")) {
        printf("  [OK]   /proc/umdp/permtab exists\n");
        ok++;
    } else {
        printf("  [MISS] /proc/umdp/permtab exists\n");
        printf("         Fix: insmod /root/shared/umdp.ko\n");
        miss++;
    }

    if (path_exists("/root/shared/umdp.ko") || path_exists("./shared/umdp.ko")) {
        printf("  [OK]   umdp.ko present on board\n");
        ok++;
    } else {
        printf("  [MISS] umdp.ko present on board\n");
        miss++;
    }

    if (timer_irq_functional_probe()) {
        printf("  [OK]   timer IRQ functional check passed\n");
        ok++;
    } else {
        printf("  [MISS] timer IRQ functional check failed\n");
        printf("         Fix: run host steps: image build-boot + image install-boot <sdX1>, then reboot\n");
        printf("         Also ensure shared binaries are synced (timer_probe required)\n");
        miss++;
    }

    printf("[kitfeup-cli] summary: %d ok, %d missing\n", ok, miss);
    return miss == 0 ? 0 : 1;
}

int board_setup(void) {
    int rc;

    if (geteuid() != 0) {
        fprintf(stderr, "[kitfeup-cli] board setup requires root. Run as root.\n");
        return 1;
    }

    printf("[kitfeup-cli] Running board first-boot setup\n");

    if (path_exists("/usr/sbin/parted") || path_exists("/sbin/parted") || command_exists("parted")) {
        rc = run_cmd("parted -s -a opt /dev/mmcblk0 \"resizepart 3 100%\" || true");
        if (rc != 0) {
            fprintf(stderr, "[kitfeup-cli] WARN: parted resize returned non-zero\n");
        }
    } else {
        fprintf(stderr, "[kitfeup-cli] WARN: parted not found, skipping partition resize\n");
    }

    if (path_exists("/sbin/resize2fs") || command_exists("resize2fs")) {
        rc = run_cmd("resize2fs /dev/mmcblk0p3 || true");
        if (rc != 0) {
            fprintf(stderr, "[kitfeup-cli] WARN: resize2fs returned non-zero\n");
        }
    } else {
        fprintf(stderr, "[kitfeup-cli] WARN: resize2fs not found, skipping filesystem resize\n");
    }

    if (path_exists("/mnt/system/blink.sh")) {
        if (path_exists("/mnt/system/blink.sh_backup")) {
            printf("[kitfeup-cli] blink script already backed up\n");
        } else {
            rc = run_cmd("mv /mnt/system/blink.sh /mnt/system/blink.sh_backup");
            if (rc != 0) {
                fprintf(stderr, "[kitfeup-cli] WARN: failed to disable blink.sh\n");
            } else {
                printf("[kitfeup-cli] disabled /mnt/system/blink.sh\n");
            }
        }
    } else {
        printf("[kitfeup-cli] /mnt/system/blink.sh not found, skipping\n");
    }

    (void) run_cmd("sync");
    printf("[kitfeup-cli] setup complete. Reboot recommended.\n");
    return 0;
}

int board_run_prog(int argc, char** argv) {
    char path[512];

    if (argc < 3) {
        fprintf(stderr, "Usage: kitfeup run <program> [args...]\n");
        return 1;
    }

    if (argv[2][0] == '/') {
        snprintf(path, sizeof(path), "%s", argv[2]);
    } else {
        snprintf(path, sizeof(path), "/root/shared/compiled/%s", argv[2]);
    }

    if (access(path, X_OK) != 0) {
        fprintf(stderr, "Program not executable or not found: %s\n", path);
        return 1;
    }

    return run_exec(path, argc, argv, 3);
}

int board_default(void) {
    return board_doctor();
}

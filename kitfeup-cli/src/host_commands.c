#include "host_commands.h"

#include "common.h"

#include <stdio.h>
#include <string.h>

void host_print_help(void) {
    puts("kitfeup (host mode)");
    puts("");
    puts("Usage:");
    puts("  kitfeup              Setup-if-needed + compile + sync");
    puts("  kitfeup doctor       Host readiness report");
    puts("  kitfeup setup        Setup prerequisites if missing");
    puts("  kitfeup compile      Build artifacts incrementally");
    puts("  kitfeup sync         Sync artifacts to board");
    puts("  kitfeup clean        Clean build artifacts");
    puts("  kitfeup image ...    Image workflows (build/download/flash)");
    puts("  (compile/default auto-sync; use --no-sync to skip)");
    puts("");
    puts("Image commands:");
    puts("  kitfeup image build");
    puts("  kitfeup image build-boot");
    puts("  kitfeup image strip-cimg <boot.sd>");
    puts("  kitfeup image download-latest [owner/repo]");
    puts("  kitfeup image flash <device> [image]");
    puts("  kitfeup image install-boot <boot-partition>");
    puts("");
    puts("Host plugins:");
    puts("  kitfeup <plugin> ...");
    puts("  Search path: kitfeup.d/host/<plugin>, ~/.config/kitfeup/plugins/host/<plugin>");
}

int host_doctor(void) {
    int ok = 0;
    int miss = 0;

    printf("[kitfeup-cli] Host doctor report\n");
    printf("  [OK]   Linux host\n");
    ok++;

    if (command_exists("nix-shell")) {
        printf("  [OK]   nix-shell available\n");
        ok++;
    } else {
        printf("  [MISS] nix-shell available\n");
        miss++;
    }

    if (path_exists("duo-buildroot-sdk-v2/host-tools/gcc/riscv64-linux-musl-x86_64/bin/riscv64-unknown-linux-musl-gcc")) {
        printf("  [OK]   cross compiler\n");
        ok++;
    } else {
        printf("  [MISS] cross compiler\n");
        miss++;
    }

    if (path_exists("sysroot/usr/lib/libnl-3.a") && path_exists("sysroot/usr/lib/libnl-genl-3.a")) {
        printf("  [OK]   sysroot libnl static libs\n");
        ok++;
    } else {
        printf("  [MISS] sysroot libnl static libs\n");
        miss++;
    }

    if (path_exists("duo-buildroot-sdk-v2/linux_5.10/build/sg2000_milkv_duos_musl_riscv64_sd/Module.symvers")) {
        printf("  [OK]   kernel build output\n");
        ok++;
    } else {
        printf("  [MISS] kernel build output\n");
        miss++;
    }

    if (path_exists("shared/umdp.ko") && path_exists("shared/compiled")) {
        printf("  [OK]   built shared artifacts\n");
        ok++;
    } else {
        printf("  [MISS] built shared artifacts\n");
        miss++;
    }

    printf("[kitfeup-cli] summary: %d ok, %d missing\n", ok, miss);
    return miss == 0 ? 0 : 1;
}

int host_setup(void) {
    return run_cmd("make setup-toolchain-if-needed build-board-image-if-needed");
}

int host_compile(void) {
    return run_cmd("make build-all");
}

int host_sync(void) {
    return run_cmd("make sync-all sync-kitfeupb");
}

int host_clean(void) {
    return run_cmd("make clean");
}

int host_image(int argc, char** argv) {
    char cmd[2048];
    const char* sub;

    if (argc < 3) {
        fprintf(stderr, "Usage: kitfeup image <build|build-boot|strip-cimg|download-latest|flash|install-boot> ...\n");
        return 1;
    }

    sub = argv[2];
    if (strcmp(sub, "build") == 0) {
        return run_cmd("make image-build");
    }
    if (strcmp(sub, "build-boot") == 0) {
        return run_cmd("make build-custom-boot");
    }
    if (strcmp(sub, "strip-cimg") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Usage: kitfeup image strip-cimg <path-to-boot.sd>\n");
            return 1;
        }
        snprintf(cmd, sizeof(cmd), "make image-strip-cimg IMAGE=\"%s\"", argv[3]);
        return run_cmd(cmd);
    }
    if (strcmp(sub, "download-latest") == 0) {
        if (argc >= 5) {
            snprintf(cmd, sizeof(cmd), "make image-download-latest IMAGE_REPO=\"%s\" IMAGE_PATTERN=\"%s\"", argv[3], argv[4]);
        } else if (argc >= 4) {
            snprintf(cmd, sizeof(cmd), "make image-download-latest IMAGE_REPO=\"%s\"", argv[3]);
        } else {
            snprintf(cmd, sizeof(cmd), "make image-download-latest");
        }
        return run_cmd(cmd);
    }
    if (strcmp(sub, "flash") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Usage: kitfeup image flash <device> [image]\n");
            return 1;
        }
        if (argc >= 5) {
            snprintf(cmd, sizeof(cmd), "make image-flash DEVICE=\"%s\" IMAGE=\"%s\"", argv[3], argv[4]);
        } else {
            snprintf(cmd, sizeof(cmd), "make image-flash DEVICE=\"%s\"", argv[3]);
        }
        return run_cmd(cmd);
    }
    if (strcmp(sub, "install-boot") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Usage: kitfeup image install-boot <boot-partition>\n");
            return 1;
        }
        snprintf(cmd, sizeof(cmd), "make image-install-boot BOOT_PARTITION=\"%s\"", argv[3]);
        return run_cmd(cmd);
    }

    fprintf(stderr, "Unknown image subcommand: %s\n", sub);
    fprintf(stderr, "Usage: kitfeup image <build|build-boot|strip-cimg|download-latest|flash|install-boot> ...\n");
    return 1;
}

int host_default(bool no_sync) {
    int rc = host_setup();
    if (rc != 0) {
        return rc;
    }
    rc = host_compile();
    if (rc != 0) {
        return rc;
    }
    if (no_sync) {
        return 0;
    }
    return host_sync();
}

#include "board_commands.h"
#include "common.h"
#include "host_commands.h"
#include "plugins.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifndef KITFEUP_BOARD_MODE
#define KITFEUP_BOARD_MODE 0
#endif

int main(int argc, char** argv) {
    const char* cmd;
    bool no_sync = has_flag(argc, argv, "--no-sync");

    if (argc <= 1) {
#if KITFEUP_BOARD_MODE
        (void) no_sync;
        return board_default();
#else
        return host_default(no_sync);
#endif
    }

    cmd = argv[1];

    if (strcmp(cmd, "help") == 0 || strcmp(cmd, "-h") == 0 || strcmp(cmd, "--help") == 0) {
#if KITFEUP_BOARD_MODE
        board_print_help();
#else
        host_print_help();
#endif
        return 0;
    }

#if KITFEUP_BOARD_MODE
    if (strcmp(cmd, "doctor") == 0) {
        return board_doctor();
    }
    if (strcmp(cmd, "setup") == 0) {
        return board_setup();
    }
    if (strcmp(cmd, "run") == 0) {
        return board_run_prog(argc, argv);
    }
#else
    if (strcmp(cmd, "doctor") == 0) {
        return host_doctor();
    }
    if (strcmp(cmd, "setup") == 0) {
        return host_setup();
    }
    if (strcmp(cmd, "compile") == 0 || strcmp(cmd, "build") == 0) {
        int rc = host_compile();
        if (rc != 0) {
            return rc;
        }
        if (no_sync) {
            return 0;
        }
        return host_sync();
    }
    if (strcmp(cmd, "sync") == 0) {
        return host_sync();
    }
    if (strcmp(cmd, "clean") == 0) {
        return host_clean();
    }
    if (strcmp(cmd, "image") == 0) {
        return host_image(argc, argv);
    }
#endif

#if KITFEUP_BOARD_MODE
    {
        int plugin_rc = try_external_plugin(cmd, argc, argv, "board");
        if (plugin_rc >= 0) {
            return plugin_rc;
        }
    }
#else
    {
        int plugin_rc = try_external_plugin(cmd, argc, argv, "host");
        if (plugin_rc >= 0) {
            return plugin_rc;
        }
    }
#endif

    fprintf(stderr, "Unknown command: %s\n", cmd);
#if KITFEUP_BOARD_MODE
    board_print_help();
#else
    host_print_help();
#endif
    return 1;
}

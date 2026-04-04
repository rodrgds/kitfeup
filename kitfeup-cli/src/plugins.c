#include "plugins.h"

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int try_external_plugin(const char* cmd, int argc, char** argv, const char* mode_name) {
    char path[512];
    const char* home = getenv("HOME");

    snprintf(path, sizeof(path), "kitfeup.d/%s/%s", mode_name, cmd);
    if (access(path, X_OK) == 0) {
        return run_exec(path, argc, argv, 2);
    }

    if (home != NULL) {
        snprintf(path, sizeof(path), "%s/.config/kitfeup/plugins/%s/%s", home, mode_name, cmd);
        if (access(path, X_OK) == 0) {
            return run_exec(path, argc, argv, 2);
        }
    }

    return -1;
}

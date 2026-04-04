#include "common.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int run_cmd(const char* cmd) {
    int rc = system(cmd);
    if (rc == -1) {
        return 1;
    }
    if (WIFEXITED(rc)) {
        return WEXITSTATUS(rc);
    }
    return 1;
}

int run_exec(const char* path, int argc, char** argv, int arg_start) {
    char** exec_argv;
    int i;
    int out_i = 1;

    exec_argv = (char**) calloc((size_t) (argc - arg_start + 2), sizeof(char*));
    if (exec_argv == NULL) {
        return 1;
    }

    exec_argv[0] = (char*) path;
    for (i = arg_start; i < argc; i++) {
        exec_argv[out_i++] = argv[i];
    }
    exec_argv[out_i] = NULL;

    pid_t pid = fork();
    if (pid < 0) {
        free(exec_argv);
        return 1;
    }
    if (pid == 0) {
        execv(path, exec_argv);
        _exit(127);
    }

    {
        int status = 0;
        if (waitpid(pid, &status, 0) < 0) {
            free(exec_argv);
            return 1;
        }
        free(exec_argv);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
    }

    return 1;
}

bool path_exists(const char* path) {
    struct stat st;
    return stat(path, &st) == 0;
}

bool file_contains(const char* path, const char* needle) {
    FILE* f;
    char line[1024];

    f = fopen(path, "r");
    if (f == NULL) {
        return false;
    }

    while (fgets(line, sizeof(line), f) != NULL) {
        if (strstr(line, needle) != NULL) {
            fclose(f);
            return true;
        }
    }

    fclose(f);
    return false;
}

bool command_exists(const char* name) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "command -v %s >/dev/null 2>&1", name);
    return run_cmd(cmd) == 0;
}

bool has_flag(int argc, char** argv, const char* flag) {
    int i;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], flag) == 0) {
            return true;
        }
    }
    return false;
}

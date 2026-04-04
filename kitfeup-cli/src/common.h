#ifndef KITFEUP_COMMON_H
#define KITFEUP_COMMON_H

#include <stdbool.h>

int run_cmd(const char* cmd);
int run_exec(const char* path, int argc, char** argv, int arg_start);
bool path_exists(const char* path);
bool file_contains(const char* path, const char* needle);
bool command_exists(const char* name);
bool has_flag(int argc, char** argv, const char* flag);

#endif

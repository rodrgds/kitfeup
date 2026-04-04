#ifndef KITFEUP_HOST_COMMANDS_H
#define KITFEUP_HOST_COMMANDS_H

#include <stdbool.h>

void host_print_help(void);
int host_doctor(void);
int host_setup(void);
int host_compile(void);
int host_sync(void);
int host_clean(void);
int host_image(int argc, char** argv);
int host_default(bool no_sync);

#endif

#ifndef KITFEUP_BOARD_COMMANDS_H
#define KITFEUP_BOARD_COMMANDS_H

void board_print_help(void);
int board_doctor(void);
int board_setup(void);
int board_run_prog(int argc, char** argv);
int board_default(void);

#endif

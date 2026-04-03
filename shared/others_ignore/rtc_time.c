/*
 * Clock Driver - Uses C time.h (hardware RTC offsets undocumented on SG2000)
 * 
 * The SG2000's RTC uses proprietary undocumented offsets.
 * Using standard C library time functions instead.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <stdbool.h>
#include <time.h>
#include "../patched-umdp/libumdp/include/umdp.h"

static volatile bool running = true;

static void signal_handler(int sig) {
    (void)sig;
    running = false;
}

int main(void) {
    printf("=== Clock Driver ===\n");
    printf("NOTE: SG2000 RTC has proprietary undocumented offsets.\n");
    printf("      Using C time.h library instead.\n\n");

    signal(SIGINT, signal_handler);

    while (running) {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);

        printf("\r%04d-%02d-%02d %02d:%02d:%02d  ",
               t->tm_year + 1900,
               t->tm_mon + 1,
               t->tm_mday,
               t->tm_hour,
               t->tm_min,
               t->tm_sec);
        fflush(stdout);

        sleep(1);
    }

    printf("\nClock driver stopped.\n");
    return 0;
}

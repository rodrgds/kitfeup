/*
 * Random Number Generator using /dev/urandom
 *
 * The SG2000 SARADC clock is gated and has no driver interface.
 * Using Linux kernel's built-in CSPRNG instead.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <time.h>

static volatile bool running = true;

static void signal_handler(int sig)
{
    (void)sig;
    running = false;
}

/* Read random bytes from kernel CSPRNG */
static uint32_t get_random(void)
{
    uint32_t val;
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0)
    {
        return rand(); /* Fallback to C rand() */
    }
    read(fd, &val, sizeof(val));
    close(fd);
    return val;
}

int main(int argc, char *argv[])
{
    int count = 0;
    int max_count = 0;

    if (argc > 1)
    {
        max_count = atoi(argv[1]);
    }

    printf("=== Random Number Generator ===\n");
    printf("Source: /dev/urandom (Linux kernel CSPRNG)\n");
    printf("Note: SG2000 has no hardware TRNG. SARADC clock is gated.\n\n");

    signal(SIGINT, signal_handler);

    /* Seed C rand() with hardware entropy */
    srand(get_random());

    while (running)
    {
        uint32_t random_val = get_random();
        printf("[%d] Random: 0x%08X (%u)\n", count, random_val, random_val);

        count++;
        if (max_count > 0 && count >= max_count)
        {
            break;
        }

        sleep(1);
    }

    printf("\nGenerated %d random numbers.\n", count);
    return 0;
}

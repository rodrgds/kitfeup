/*
 * Fibonacci in RISC-V assembly
 */

#include <stdio.h>

extern int fib_asm(int n);

int main(void)
{
    printf("=== RISC-V Assembly Fibonacci ===\n\n");

    for (int i = 0; i < 20; i++)
    {
        int result = fib_asm(i);
        printf("fib(%2d) = %d\n", i, result);
    }

    return 0;
}

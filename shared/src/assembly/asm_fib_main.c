/*
 * Fibonacci in RISC-V assembly test
 */

#include <stdio.h>

extern int fib_asm(int n);

int main(void)
{
    int expected[] = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377};
    int pass = 1;

    printf("=== RISC-V Assembly Fibonacci Test ===\n\n");

    for (int i = 0; i < 15; i++) {
        int result = fib_asm(i);
        printf("fib(%2d) = %4d", i, result);
        if (result == expected[i]) {
            printf("  OK\n");
        } else {
            printf("  FAIL (expected %d)\n", expected[i]);
            pass = 0;
        }
    }

    printf("\n");
    if (pass) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("Some tests failed.\n");
        return 1;
    }
}

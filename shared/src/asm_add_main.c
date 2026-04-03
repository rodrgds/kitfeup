/*
 * Simple test: call an assembly function that adds two numbers
 */

#include <stdio.h>

extern int add_numbers_asm(int a, int b);

int main(void)
{
    int a = 17;
    int b = 25;

    int result = add_numbers_asm(a, b);

    printf("Assembly add test: %d + %d = %d\n", a, b, result);
}

.global fib_asm
.type fib_asm, @function

// Function Signature: int fib_asm(int n);
// a0 = n (which fibonacci number to compute)
// return value in a0
fib_asm:
    // Handle base cases: fib(0) = 0, fib(1) = 1
    li t0, 1
    ble a0, t0, .Lbase_case

    // Iterative fibonacci:
    // t1 = fib(i-2), t2 = fib(i-1), t3 = current result
    li t1, 0          // fib(0)
    li t2, 1          // fib(1)
    li t3, 2          // loop counter starts at 2

.Lloop:
    add t0, t1, t2    // t0 = fib(i-2) + fib(i-1)
    mv t1, t2         // shift: fib(i-2) = old fib(i-1)
    mv t2, t0         // shift: fib(i-1) = new result
    addi t3, t3, 1    // counter++
    ble t3, a0, .Lloop  // if counter <= n, continue

    mv a0, t2         // result in a0
    ret

.Lbase_case:
    // a0 is already 0 or 1, just return it
    ret

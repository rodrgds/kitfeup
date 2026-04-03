.global add_numbers_asm
.type add_numbers_asm, @function

// Function Signature: int add_numbers_asm(int a, int b);
// a0 = first argument (a)
// a1 = second argument (b)
// return value in a0
add_numbers_asm:
    add a0, a0, a1    // a0 = a0 + a1
    ret               // return to caller (result already in a0)

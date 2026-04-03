.global toggle_led_asm
.type toggle_led_asm, @function

// Function Signature: void toggle_led_asm(volatile uint32_t* gpio_base, uint32_t pin);
// a0 = gpio_base (Virtual address provided by Linux/UMDP)
// a1 = pin (29)
toggle_led_asm:
    // 1. Create the bitmask for the pin: (1 << pin)
    li t1, 1              // Load immediate 1 into temporary register t1
    sll t1, t1, a1        // Shift left logical: t1 = 1 << a1. Now t1 holds our mask for Pin 29

    // 2. Set GPIO Direction to Output (Register Offset 0x04)
    lw t0, 4(a0)          // Load Word: read the current Direction Register (base + 4 bytes)
    or t0, t0, t1         // Bitwise OR: force the 29th bit to 1 (Output) without changing other pins
    sw t0, 4(a0)          // Store Word: write the new value back to the Direction Register

    // 3. Toggle the GPIO Data (Register Offset 0x00)
    lw t0, 0(a0)          // Load Word: read the current Data Register (base + 0 bytes)
    xor t0, t0, t1        // Bitwise XOR: flip the 29th bit (1 becomes 0, 0 becomes 1)
    sw t0, 0(a0)          // Store Word: write the toggled value back to the Data Register

    // 4. Return to C
    ret                   // Return to caller

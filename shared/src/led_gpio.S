.global led_set_output_asm
.type led_set_output_asm, @function

// void led_set_output_asm(volatile uint32_t* gpio_base, uint32_t pin)
// a0 = gpio_base
// a1 = pin
led_set_output_asm:
    li t1, 1
    sll t1, t1, a1

    // GPIO DIR register at offset 0x04
    lw t0, 4(a0)
    or t0, t0, t1
    sw t0, 4(a0)
    ret


.global led_write_asm
.type led_write_asm, @function

// void led_write_asm(volatile uint32_t* gpio_base, uint32_t pin, uint32_t on)
// a0 = gpio_base
// a1 = pin
// a2 = on (0 or non-zero)
led_write_asm:
    li t1, 1
    sll t1, t1, a1

    // GPIO DATA register at offset 0x00
    lw t0, 0(a0)
    // Check if 'on' is zero
    beqz a2, .Lclear
    // Set the bit to turn the LED on
    or t0, t0, t1
    sw t0, 0(a0)
    ret

.Lclear:
    // To clear the bit, we need to AND with the inverse of t1
    xori t1, t1, -1
    and t0, t0, t1
    sw t0, 0(a0)
    ret

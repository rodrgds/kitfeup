ROOT_DIR := $(abspath ..)
SRC_DIR := src
OUTDIR := compiled

CC := $(ROOT_DIR)/duo-buildroot-sdk-v2/host-tools/gcc/riscv64-linux-musl-x86_64/bin/riscv64-unknown-linux-musl-gcc

INCLUDES := -I $(ROOT_DIR)/patched-umdp/umdp/ -I $(ROOT_DIR)/patched-umdp/libumdp/include/ -I $(SRC_DIR)
INCLUDES += -I $(SRC_DIR)/timer -I $(SRC_DIR)/led -I $(SRC_DIR)/rtc
LIBDIRS := -L $(ROOT_DIR)/patched-umdp/libumdp/build/ -L $(ROOT_DIR)/sysroot/usr/lib/
LIBS := -lumdp -lnl-genl-3 -lnl-3

CFLAGS := -Wall -Wextra -std=gnu11
CFLAGS += -D_POSIX_C_SOURCE=200809L
CFLAGS += -D_DEFAULT_SOURCE

PROGRAMS := \
	blink \
	blink_umdp \
	blink_timer \
	uptime \
	timer_wait \
	timer_probe \
	timer_multi \
	asm_led \
	asm_add \
	asm_fib

all: $(OUTDIR) $(PROGRAMS)

$(OUTDIR):
	mkdir -p $(OUTDIR)

blink: $(SRC_DIR)/blink.c | $(OUTDIR)
	$(CC) $(CFLAGS) $(INCLUDES) $< $(LIBDIRS) $(LIBS) -o $(OUTDIR)/$@

blink_umdp: $(SRC_DIR)/blink_umdp.c | $(OUTDIR)
	$(CC) $(CFLAGS) $(INCLUDES) $< $(LIBDIRS) $(LIBS) -o $(OUTDIR)/$@

blink_timer: $(SRC_DIR)/blink_timer.c $(SRC_DIR)/timer/timer_irq.c $(SRC_DIR)/led/led_gpio.c | $(OUTDIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(SRC_DIR)/blink_timer.c $(SRC_DIR)/timer/timer_irq.c $(SRC_DIR)/led/led_gpio.c $(LIBDIRS) $(LIBS) -o $(OUTDIR)/$@

uptime: $(SRC_DIR)/uptime.c $(SRC_DIR)/rtc/rtc.c | $(OUTDIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(SRC_DIR)/uptime.c $(SRC_DIR)/rtc/rtc.c $(LIBDIRS) $(LIBS) -o $(OUTDIR)/$@

timer_wait: $(SRC_DIR)/timer_wait.c $(SRC_DIR)/timer/timer_irq.c $(SRC_DIR)/led/led_gpio.c | $(OUTDIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(SRC_DIR)/timer_wait.c $(SRC_DIR)/timer/timer_irq.c $(SRC_DIR)/led/led_gpio.c $(LIBDIRS) $(LIBS) -o $(OUTDIR)/$@

timer_probe: $(SRC_DIR)/timer_probe.c $(SRC_DIR)/timer/timer_irq.c | $(OUTDIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(SRC_DIR)/timer_probe.c $(SRC_DIR)/timer/timer_irq.c $(LIBDIRS) $(LIBS) -o $(OUTDIR)/$@

timer_multi: $(SRC_DIR)/timer_multi.c $(SRC_DIR)/timer/timer_multi.c $(SRC_DIR)/timer/timer_irq.c | $(OUTDIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(SRC_DIR)/timer_multi.c $(SRC_DIR)/timer/timer_multi.c $(SRC_DIR)/timer/timer_irq.c $(LIBDIRS) $(LIBS) -o $(OUTDIR)/$@

asm_led: $(SRC_DIR)/asm_led_main.c $(SRC_DIR)/led_gpio.S | $(OUTDIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(SRC_DIR)/asm_led_main.c $(SRC_DIR)/led_gpio.S $(LIBDIRS) $(LIBS) -o $(OUTDIR)/$@

asm_add: $(SRC_DIR)/asm_add_main.c $(SRC_DIR)/add.S | $(OUTDIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(SRC_DIR)/asm_add_main.c $(SRC_DIR)/add.S -o $(OUTDIR)/$@

asm_fib: $(SRC_DIR)/asm_fib_main.c $(SRC_DIR)/fib.S | $(OUTDIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(SRC_DIR)/asm_fib_main.c $(SRC_DIR)/fib.S -o $(OUTDIR)/$@

clean:
	rm -rf $(OUTDIR)

.PHONY: all clean $(PROGRAMS)

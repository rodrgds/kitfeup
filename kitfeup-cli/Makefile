ROOT_DIR := $(abspath ..)
SRCS := src/main.c src/common.c src/plugins.c src/host_commands.c src/board_commands.c
BIN_DIR := bin
HOST_BIN := $(BIN_DIR)/kitfeup
BOARD_BIN := $(BIN_DIR)/kitfeupb

CC ?= cc
CFLAGS ?= -O2 -Wall -Wextra -std=c11

BOARD_CC := $(ROOT_DIR)/duo-buildroot-sdk-v2/host-tools/gcc/riscv64-linux-musl-x86_64/bin/riscv64-unknown-linux-musl-gcc

.PHONY: all host board clean

all: host

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

host: $(HOST_BIN)

$(HOST_BIN): $(SRCS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(SRCS)

board: $(BOARD_BIN)

$(BOARD_BIN): $(SRCS) | $(BIN_DIR)
	@if [ ! -x "$(BOARD_CC)" ]; then \
		echo "Missing cross compiler: $(BOARD_CC)"; \
		exit 1; \
	fi
	$(BOARD_CC) $(CFLAGS) -DKITFEUP_BOARD_MODE=1 -static -o $@ $(SRCS)

clean:
	rm -rf $(BIN_DIR)

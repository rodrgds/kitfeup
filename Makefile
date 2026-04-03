ROOT_DIR := $(abspath .)
SHELL_NIX := $(ROOT_DIR)/shell.nix
NIX_SHELL := nix-shell $(SHELL_NIX)

BOARD_USER ?= root
BOARD_HOST ?= 192.168.42.1
BOARD_PASS ?= milkv
BOARD_SHARED_DIR ?= /root/shared

KERNEL_BUILD_DIR ?= $(ROOT_DIR)/duo-buildroot-sdk-v2/linux_5.10/build/sg2000_milkv_duos_musl_riscv64_sd
UMDP_DIR ?= $(ROOT_DIR)/patched-umdp/umdp

CROSS_PREFIX ?= $(ROOT_DIR)/duo-buildroot-sdk-v2/host-tools/gcc/riscv64-linux-musl-x86_64/bin/riscv64-unknown-linux-musl-

.PHONY: help setup-toolchain build-libumdp build-umdp-ko build-shared build-all build-board-image sync-compiled sync-umdp-ko sync-all test-smoke dump-runtime-dtb decompile-dtb clean

help:
	@printf "KitFEUP automation targets:\n"
	@printf "  make setup-toolchain - Clone host-tools and build libnl sysroot\n"
	@printf "  make build-libumdp   - Build libumdp static library\n"
	@printf "  make build-umdp-ko   - Build UMDP kernel module (umdp.ko)\n"
	@printf "  make build-shared    - Build all shared user programs into shared/compiled\n"
	@printf "  make build-all       - Build libumdp + umdp.ko + shared programs\n"
	@printf "  make build-board-image - Build SDK boot artifacts (boot.sd/itb/fip)\n"
	@printf "  make sync-compiled   - Sync shared/compiled to board\n"
	@printf "  make sync-umdp-ko    - Sync shared/umdp.ko to board\n"
	@printf "  make sync-all        - Sync compiled binaries + umdp.ko to board\n"
	@printf "  make test-smoke      - Run short smoke tests on board\n"
	@printf "  make dump-runtime-dtb - Copy running board DTB to install output\n"
	@printf "  make decompile-dtb    - Decompile runtime/generated DTBs to DTS\n"
	@printf "\nConfigurable vars:\n"
	@printf "  BOARD_HOST=%s BOARD_USER=%s BOARD_PASS=%s BOARD_SHARED_DIR=%s\n" "$(BOARD_HOST)" "$(BOARD_USER)" "$(BOARD_PASS)" "$(BOARD_SHARED_DIR)"

setup-toolchain:
	env ROOT_DIR="$(ROOT_DIR)" $(NIX_SHELL) < "$(ROOT_DIR)/scripts/nix/setup-toolchain.sh"

build-libumdp:
	env ROOT_DIR="$(ROOT_DIR)" $(NIX_SHELL) < "$(ROOT_DIR)/scripts/nix/build-libumdp.sh"

build-umdp-ko:
	env ROOT_DIR="$(ROOT_DIR)" KERNEL_BUILD_DIR="$(KERNEL_BUILD_DIR)" UMDP_DIR="$(UMDP_DIR)" CROSS_PREFIX="$(CROSS_PREFIX)" $(NIX_SHELL) < "$(ROOT_DIR)/scripts/nix/build-umdp-ko.sh"

build-shared:
	env ROOT_DIR="$(ROOT_DIR)" $(NIX_SHELL) < "$(ROOT_DIR)/scripts/nix/build-shared.sh"

build-all: build-libumdp build-umdp-ko build-shared

build-board-image:
	env ROOT_DIR="$(ROOT_DIR)" $(NIX_SHELL) < "$(ROOT_DIR)/scripts/nix/build-board-image.sh"
	@echo "Stripping CIMG header if present..."
	@"$(ROOT_DIR)/scripts/nix/strip-cimg.sh" "$(ROOT_DIR)/duo-buildroot-sdk-v2/install/soc_sg2000_milkv_duos_musl_riscv64_sd/boot.sd"

sync-compiled:
	env ROOT_DIR="$(ROOT_DIR)" BOARD_PASS="$(BOARD_PASS)" BOARD_USER="$(BOARD_USER)" BOARD_HOST="$(BOARD_HOST)" BOARD_SHARED_DIR="$(BOARD_SHARED_DIR)" $(NIX_SHELL) < "$(ROOT_DIR)/scripts/nix/sync-compiled.sh"

sync-umdp-ko:
	env ROOT_DIR="$(ROOT_DIR)" BOARD_PASS="$(BOARD_PASS)" BOARD_USER="$(BOARD_USER)" BOARD_HOST="$(BOARD_HOST)" BOARD_SHARED_DIR="$(BOARD_SHARED_DIR)" $(NIX_SHELL) < "$(ROOT_DIR)/scripts/nix/sync-umdp-ko.sh"

sync-all: sync-compiled sync-umdp-ko

test-smoke:
	env ROOT_DIR="$(ROOT_DIR)" BOARD_PASS="$(BOARD_PASS)" BOARD_USER="$(BOARD_USER)" BOARD_HOST="$(BOARD_HOST)" BOARD_SHARED_DIR="$(BOARD_SHARED_DIR)" $(NIX_SHELL) < "$(ROOT_DIR)/scripts/nix/test-smoke.sh"

dump-runtime-dtb:
	env ROOT_DIR="$(ROOT_DIR)" BOARD_PASS="$(BOARD_PASS)" BOARD_USER="$(BOARD_USER)" BOARD_HOST="$(BOARD_HOST)" BOARD_SHARED_DIR="$(BOARD_SHARED_DIR)" $(NIX_SHELL) < "$(ROOT_DIR)/scripts/nix/dump-runtime-dtb.sh"

decompile-dtb:
	env ROOT_DIR="$(ROOT_DIR)" $(NIX_SHELL) < "$(ROOT_DIR)/scripts/nix/decompile-dtb.sh"

clean:
	$(MAKE) -C $(ROOT_DIR)/shared clean
	rm -f $(ROOT_DIR)/shared/umdp.ko

ROOT_DIR := $(abspath .)
SHELL_NIX := $(ROOT_DIR)/shell.nix
NIX_SHELL := nix-shell $(SHELL_NIX)

.DEFAULT_GOAL := do

BOARD_USER ?= root
BOARD_HOST ?= 192.168.42.1
BOARD_PASS ?= milkv
BOARD_SHARED_DIR ?= /root/shared

KERNEL_BUILD_DIR ?= $(ROOT_DIR)/duo-buildroot-sdk-v2/linux_5.10/build/sg2000_milkv_duos_musl_riscv64_sd
UMDP_DIR ?= $(ROOT_DIR)/patched-umdp/umdp

CROSS_PREFIX ?= $(ROOT_DIR)/duo-buildroot-sdk-v2/host-tools/gcc/riscv64-linux-musl-x86_64/bin/riscv64-unknown-linux-musl-

.PHONY: help check-nix setup-toolchain setup-toolchain-if-needed build-libumdp build-umdp-ko build-shared build-all build-board-image build-custom-boot build-board-image-if-needed image-build image-strip-cimg image-download-latest image-flash image-install-boot sync-compiled sync-umdp-ko sync-kitfeupb sync-all test-smoke dump-runtime-dtb decompile-dtb do do-full clean kitfeup-cli

help:
	@printf "KitFEUP:\n"
	@printf "  ./kitfeup           - Primary CLI (setup/build/sync/image)\n"
	@printf "  make kitfeup-cli    - Build host + board CLI binaries\n"
	@printf "  make clean          - Clean build artifacts\n"
	@printf "\nInternal targets (use ./kitfeup instead):\n"
	@printf "  All build/sync/image logic is automated by the CLI.\n"
	@printf "  Remaining targets are for backward compatibility only.\n"

check-nix:
	@if [ "$(shell uname -s)" != "Linux" ]; then \
		echo "[check-nix] This project build environment currently supports Linux only (buildFHSEnv)."; \
		exit 1; \
	fi
	@if ! command -v nix-shell >/dev/null 2>&1; then \
		echo "[check-nix] nix-shell not found."; \
		echo "[check-nix] Install Nix first:"; \
		echo "  sh <(curl -L https://nixos.org/nix/install) --daemon"; \
		echo "[check-nix] Then restart your shell and re-run 'make'."; \
		exit 1; \
	fi

setup-toolchain: check-nix
	env ROOT_DIR="$(ROOT_DIR)" $(NIX_SHELL) < "$(ROOT_DIR)/kitfeup-cli/scripts/setup-toolchain.sh"

setup-toolchain-if-needed: check-nix
	@if [ ! -x "$(ROOT_DIR)/duo-buildroot-sdk-v2/host-tools/gcc/riscv64-linux-musl-x86_64/bin/riscv64-unknown-linux-musl-gcc" ] \
	 || [ ! -f "$(ROOT_DIR)/sysroot/usr/lib/libnl-3.a" ] \
	 || [ ! -f "$(ROOT_DIR)/sysroot/usr/lib/libnl-genl-3.a" ] \
	 || [ ! -f "$(ROOT_DIR)/sysroot/usr/include/netlink/netlink.h" ]; then \
		echo "[do] Toolchain/sysroot missing pieces; running setup-toolchain..."; \
		$(MAKE) setup-toolchain; \
	else \
		echo "[do] Toolchain/sysroot already ready"; \
	fi

build-libumdp: check-nix
	env ROOT_DIR="$(ROOT_DIR)" $(NIX_SHELL) < "$(ROOT_DIR)/kitfeup-cli/scripts/build-libumdp.sh"

build-umdp-ko: check-nix
	env ROOT_DIR="$(ROOT_DIR)" KERNEL_BUILD_DIR="$(KERNEL_BUILD_DIR)" UMDP_DIR="$(UMDP_DIR)" CROSS_PREFIX="$(CROSS_PREFIX)" $(NIX_SHELL) < "$(ROOT_DIR)/kitfeup-cli/scripts/build-umdp-ko.sh"

build-shared: check-nix
	env ROOT_DIR="$(ROOT_DIR)" $(NIX_SHELL) < "$(ROOT_DIR)/kitfeup-cli/scripts/build-shared.sh"

build-all: build-libumdp build-umdp-ko build-shared

build-board-image: check-nix
	env ROOT_DIR="$(ROOT_DIR)" $(NIX_SHELL) < "$(ROOT_DIR)/kitfeup-cli/scripts/build-board-image.sh"
	@echo "Stripping CIMG header if present..."
	@"$(ROOT_DIR)/kitfeup-cli/scripts/strip-cimg.sh" "$(ROOT_DIR)/duo-buildroot-sdk-v2/install/soc_sg2000_milkv_duos_musl_riscv64_sd/boot.sd"

image-build: build-board-image

build-custom-boot: check-nix
	env ROOT_DIR="$(ROOT_DIR)" $(NIX_SHELL) < "$(ROOT_DIR)/kitfeup-cli/scripts/build-custom-boot.sh"

image-strip-cimg:
	@if [ -z "$(IMAGE)" ]; then \
		echo "Usage: make image-strip-cimg IMAGE=/path/to/boot.sd"; \
		exit 1; \
	fi
	"$(ROOT_DIR)/kitfeup-cli/scripts/strip-cimg.sh" "$(IMAGE)"

image-download-latest: check-nix
	env ROOT_DIR="$(ROOT_DIR)" IMAGE_REPO="$(IMAGE_REPO)" IMAGE_PATTERN="$(IMAGE_PATTERN)" DOWNLOAD_DIR="$(ROOT_DIR)/downloads" $(NIX_SHELL) < "$(ROOT_DIR)/kitfeup-cli/scripts/download-latest-image.sh"

image-flash:
	@if [ -z "$(DEVICE)" ]; then \
		echo "Usage: make image-flash DEVICE=/dev/sdX [IMAGE=/path/to/image.img|.img.xz]"; \
		exit 1; \
	fi
	env ROOT_DIR="$(ROOT_DIR)" DEVICE="$(DEVICE)" IMAGE="$(IMAGE)" bash "$(ROOT_DIR)/kitfeup-cli/scripts/flash-image.sh"

image-install-boot:
	@if [ -z "$(BOOT_PARTITION)" ]; then \
		echo "Usage: make image-install-boot BOOT_PARTITION=/dev/sdX1"; \
		exit 1; \
	fi
	env ROOT_DIR="$(ROOT_DIR)" BOOT_PARTITION="$(BOOT_PARTITION)" MOUNT_POINT="$(MOUNT_POINT)" bash "$(ROOT_DIR)/kitfeup-cli/scripts/install-boot-sd.sh"

build-board-image-if-needed:
	@if [ ! -d "$(KERNEL_BUILD_DIR)" ] \
	 || [ ! -f "$(KERNEL_BUILD_DIR)/Module.symvers" ] \
	 || [ ! -f "$(ROOT_DIR)/duo-buildroot-sdk-v2/install/soc_sg2000_milkv_duos_musl_riscv64_sd/boot.sd" ]; then \
		echo "[do] Kernel/boot artifacts missing; running build-board-image..."; \
		$(MAKE) build-board-image; \
	else \
		echo "[do] Kernel/boot artifacts already present"; \
	fi

sync-compiled: check-nix
	env ROOT_DIR="$(ROOT_DIR)" BOARD_PASS="$(BOARD_PASS)" BOARD_USER="$(BOARD_USER)" BOARD_HOST="$(BOARD_HOST)" BOARD_SHARED_DIR="$(BOARD_SHARED_DIR)" $(NIX_SHELL) < "$(ROOT_DIR)/kitfeup-cli/scripts/sync-compiled.sh"

sync-umdp-ko: check-nix
	env ROOT_DIR="$(ROOT_DIR)" BOARD_PASS="$(BOARD_PASS)" BOARD_USER="$(BOARD_USER)" BOARD_HOST="$(BOARD_HOST)" BOARD_SHARED_DIR="$(BOARD_SHARED_DIR)" $(NIX_SHELL) < "$(ROOT_DIR)/kitfeup-cli/scripts/sync-umdp-ko.sh"

sync-kitfeupb: check-nix
	$(MAKE) -C $(ROOT_DIR)/kitfeup-cli board
	env ROOT_DIR="$(ROOT_DIR)" BOARD_PASS="$(BOARD_PASS)" BOARD_USER="$(BOARD_USER)" BOARD_HOST="$(BOARD_HOST)" BOARD_SHARED_DIR="$(BOARD_SHARED_DIR)" $(NIX_SHELL) < "$(ROOT_DIR)/kitfeup-cli/scripts/sync-kitfeupb.sh"

sync-all: check-nix
	env ROOT_DIR="$(ROOT_DIR)" BOARD_PASS="$(BOARD_PASS)" BOARD_USER="$(BOARD_USER)" BOARD_HOST="$(BOARD_HOST)" BOARD_SHARED_DIR="$(BOARD_SHARED_DIR)" $(NIX_SHELL) < "$(ROOT_DIR)/kitfeup-cli/scripts/sync-all.sh"

test-smoke:
	env ROOT_DIR="$(ROOT_DIR)" BOARD_PASS="$(BOARD_PASS)" BOARD_USER="$(BOARD_USER)" BOARD_HOST="$(BOARD_HOST)" BOARD_SHARED_DIR="$(BOARD_SHARED_DIR)" $(NIX_SHELL) < "$(ROOT_DIR)/kitfeup-cli/scripts/test-smoke.sh"

dump-runtime-dtb:
	env ROOT_DIR="$(ROOT_DIR)" BOARD_PASS="$(BOARD_PASS)" BOARD_USER="$(BOARD_USER)" BOARD_HOST="$(BOARD_HOST)" BOARD_SHARED_DIR="$(BOARD_SHARED_DIR)" $(NIX_SHELL) < "$(ROOT_DIR)/kitfeup-cli/scripts/dump-runtime-dtb.sh"

decompile-dtb:
	env ROOT_DIR="$(ROOT_DIR)" $(NIX_SHELL) < "$(ROOT_DIR)/kitfeup-cli/scripts/decompile-dtb.sh"

do: setup-toolchain-if-needed build-board-image-if-needed
	@echo "[do] Incremental builds (no forced clean)..."
	env ROOT_DIR="$(ROOT_DIR)" $(NIX_SHELL) < "$(ROOT_DIR)/kitfeup-cli/scripts/build-libumdp.sh"
	env ROOT_DIR="$(ROOT_DIR)" KERNEL_BUILD_DIR="$(KERNEL_BUILD_DIR)" UMDP_DIR="$(UMDP_DIR)" CROSS_PREFIX="$(CROSS_PREFIX)" FULL_CLEAN=0 $(NIX_SHELL) < "$(ROOT_DIR)/kitfeup-cli/scripts/build-umdp-ko.sh"
	env ROOT_DIR="$(ROOT_DIR)" FULL_CLEAN=0 $(NIX_SHELL) < "$(ROOT_DIR)/kitfeup-cli/scripts/build-shared.sh"
	$(MAKE) sync-all sync-kitfeupb

do-full:
	@echo "[do-full] Full setup + clean rebuild + sync-all..."
	$(MAKE) setup-toolchain
	$(MAKE) build-board-image
	env ROOT_DIR="$(ROOT_DIR)" $(NIX_SHELL) < "$(ROOT_DIR)/kitfeup-cli/scripts/build-libumdp.sh"
	env ROOT_DIR="$(ROOT_DIR)" KERNEL_BUILD_DIR="$(KERNEL_BUILD_DIR)" UMDP_DIR="$(UMDP_DIR)" CROSS_PREFIX="$(CROSS_PREFIX)" FULL_CLEAN=1 $(NIX_SHELL) < "$(ROOT_DIR)/kitfeup-cli/scripts/build-umdp-ko.sh"
	env ROOT_DIR="$(ROOT_DIR)" FULL_CLEAN=1 $(NIX_SHELL) < "$(ROOT_DIR)/kitfeup-cli/scripts/build-shared.sh"
	$(MAKE) sync-all sync-kitfeupb

kitfeup-cli: setup-toolchain-if-needed
	$(MAKE) -C $(ROOT_DIR)/kitfeup-cli host board
	@if [ "$(NO_SYNC)" != "1" ]; then \
		$(MAKE) sync-kitfeupb; \
	else \
		echo "[kitfeup-cli] NO_SYNC=1 set, skipping board sync"; \
	fi

clean:
	$(MAKE) -C $(ROOT_DIR)/shared clean
	rm -f $(ROOT_DIR)/shared/umdp.ko
	rm -rf $(ROOT_DIR)/patched-umdp/libumdp/build
	@if [ -d "$(KERNEL_BUILD_DIR)" ]; then \
		$(MAKE) -C "$(KERNEL_BUILD_DIR)" M="$(UMDP_DIR)" ARCH=riscv CROSS_COMPILE="$(CROSS_PREFIX)" clean; \
	fi

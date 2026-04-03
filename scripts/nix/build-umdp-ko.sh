#!/usr/bin/env bash
set -euo pipefail

make -C "$KERNEL_BUILD_DIR" M="$UMDP_DIR" ARCH=riscv CROSS_COMPILE="$CROSS_PREFIX" clean
make -C "$KERNEL_BUILD_DIR" M="$UMDP_DIR" ARCH=riscv CROSS_COMPILE="$CROSS_PREFIX" modules
cp "$UMDP_DIR/umdp.ko" "$ROOT_DIR/shared/umdp.ko"

#!/usr/bin/env bash
set -euo pipefail

FULL_CLEAN="${FULL_CLEAN:-0}"

if [ "$FULL_CLEAN" = "1" ]; then
  make -C "$KERNEL_BUILD_DIR" M="$UMDP_DIR" ARCH=riscv CROSS_COMPILE="$CROSS_PREFIX" clean
fi

make -C "$KERNEL_BUILD_DIR" M="$UMDP_DIR" ARCH=riscv CROSS_COMPILE="$CROSS_PREFIX" modules
cp "$UMDP_DIR/umdp.ko" "$ROOT_DIR/shared/umdp.ko"

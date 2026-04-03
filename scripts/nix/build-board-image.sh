#!/usr/bin/env bash
set -eo pipefail

ROOT_DIR="${ROOT_DIR:-$(pwd)}"
SDK_DIR="${SDK_DIR:-$ROOT_DIR/duo-buildroot-sdk-v2}"
BOARD_DEFCONFIG="${BOARD_DEFCONFIG:-milkv-duos-musl-riscv64-sd}"

cd "$SDK_DIR"

source build/envsetup_milkv.sh "$BOARD_DEFCONFIG"

build_uboot
build_kernel

# Build Buildroot rootfs and pack a full SD image (.img) when possible.
make -C build br-rootfs-pack
pack_sd_image

if [ -n "${OUTPUT_DIR:-}" ]; then
  echo "[build-board-image] OUTPUT_DIR=$OUTPUT_DIR"
  find "$OUTPUT_DIR" -maxdepth 3 -type f \( -name '*.img' -o -name '*.itb' -o -name 'boot.*' -o -name 'fip*.bin' \) -print || true
fi

BOOT_SD_PATH="$SDK_DIR/ramdisk/build/sg2000_milkv_duos_musl_riscv64_sd/workspace/boot.sd"
if [ -f "$BOOT_SD_PATH" ]; then
  echo "[build-board-image] BOOT_SD=$BOOT_SD_PATH"
fi

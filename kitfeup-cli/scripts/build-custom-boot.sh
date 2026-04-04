#!/usr/bin/env bash
set -eo pipefail

ROOT_DIR="${ROOT_DIR:-$(pwd)}"
SDK_DIR="${SDK_DIR:-$ROOT_DIR/duo-buildroot-sdk-v2}"
BOARD_DEFCONFIG="${BOARD_DEFCONFIG:-milkv-duos-musl-riscv64-sd}"

cd "$SDK_DIR"
source build/envsetup_milkv.sh "$BOARD_DEFCONFIG"

build_uboot
build_kernel

BOOT_SD_PATH="$SDK_DIR/install/soc_sg2000_milkv_duos_musl_riscv64_sd/boot.sd"
ALT_BOOT_SD_PATH="$SDK_DIR/ramdisk/build/sg2000_milkv_duos_musl_riscv64_sd/workspace/boot.sd"

if [ -f "$BOOT_SD_PATH" ]; then
  :
elif [ -f "$ALT_BOOT_SD_PATH" ]; then
  BOOT_SD_PATH="$ALT_BOOT_SD_PATH"
else
  echo "[build-custom-boot] boot.sd was not generated" >&2
  exit 1
fi

echo "[build-custom-boot] boot.sd ready: $BOOT_SD_PATH"

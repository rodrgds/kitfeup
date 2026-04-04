#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="${ROOT_DIR:-$(pwd)}"
BOOT_PARTITION="${BOOT_PARTITION:-}"
MOUNT_POINT="${MOUNT_POINT:-/tmp/kitfeup-boot-mount}"
SDK_BOOT_SD="$ROOT_DIR/duo-buildroot-sdk-v2/install/soc_sg2000_milkv_duos_musl_riscv64_sd/boot.sd"
ALT_BOOT_SD="$ROOT_DIR/duo-buildroot-sdk-v2/ramdisk/build/sg2000_milkv_duos_musl_riscv64_sd/workspace/boot.sd"

if [ -z "$BOOT_PARTITION" ]; then
  echo "[image-install-boot] BOOT_PARTITION is required (e.g., /dev/sdX1)" >&2
  exit 1
fi

if [ ! -b "$BOOT_PARTITION" ]; then
  echo "[image-install-boot] Not a block device: $BOOT_PARTITION" >&2
  exit 1
fi

if [ ! -f "$SDK_BOOT_SD" ] && [ -f "$ALT_BOOT_SD" ]; then
  SDK_BOOT_SD="$ALT_BOOT_SD"
fi

if [ ! -f "$SDK_BOOT_SD" ]; then
  echo "[image-install-boot] Missing boot.sd. Build first with: make image-build" >&2
  exit 1
fi

echo "[image-install-boot] ensuring boot.sd has no CIMG header"
bash "$ROOT_DIR/kitfeup-cli/scripts/strip-cimg.sh" "$SDK_BOOT_SD"

echo "[image-install-boot] mounting $BOOT_PARTITION at $MOUNT_POINT"
sudo mkdir -p "$MOUNT_POINT"
sudo mount "$BOOT_PARTITION" "$MOUNT_POINT"

echo "[image-install-boot] copying boot.sd"
sudo cp "$SDK_BOOT_SD" "$MOUNT_POINT/boot.sd"

echo "[image-install-boot] verifying copied boot.sd"
if ! sudo test -f "$MOUNT_POINT/boot.sd"; then
  echo "[image-install-boot] failed to copy boot.sd" >&2
  sudo umount "$MOUNT_POINT" || true
  exit 1
fi
SRC_SIZE=$(stat -c%s "$SDK_BOOT_SD")
DST_SIZE=$(sudo stat -c%s "$MOUNT_POINT/boot.sd")
echo "[image-install-boot] boot.sd size: src=$SRC_SIZE dst=$DST_SIZE"

echo "[image-install-boot] unmounting"
sudo umount "$MOUNT_POINT"
sync

echo "[image-install-boot] done"

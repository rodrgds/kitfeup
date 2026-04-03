#!/usr/bin/env bash
set -euo pipefail

OUT_DIR="$ROOT_DIR/duo-buildroot-sdk-v2/install/soc_sg2000_milkv_duos_musl_riscv64_sd"
mkdir -p "$OUT_DIR"

echo "[dump-runtime-dtb] dumping /sys/firmware/fdt from $BOARD_USER@$BOARD_HOST"
sshpass -p "$BOARD_PASS" ssh -n -o StrictHostKeyChecking=no "$BOARD_USER@$BOARD_HOST" \
  "cat /sys/firmware/fdt > /tmp/stock-runtime.dtb && ls -lh /tmp/stock-runtime.dtb"

echo "[dump-runtime-dtb] copying DTB to host"
sshpass -p "$BOARD_PASS" scp -o StrictHostKeyChecking=no \
  "$BOARD_USER@$BOARD_HOST:/tmp/stock-runtime.dtb" \
  "$OUT_DIR/stock-runtime.dtb"

echo "[dump-runtime-dtb] done: $OUT_DIR/stock-runtime.dtb"

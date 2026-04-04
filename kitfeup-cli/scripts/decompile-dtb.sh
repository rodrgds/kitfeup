#!/usr/bin/env bash
set -euo pipefail

OUT_DIR="$ROOT_DIR/duo-buildroot-sdk-v2/install/soc_sg2000_milkv_duos_musl_riscv64_sd"
RUNTIME_DTB="$OUT_DIR/stock-runtime.dtb"
GENERATED_DTB="$ROOT_DIR/duo-buildroot-sdk-v2/ramdisk/build/sg2000_milkv_duos_musl_riscv64_sd/workspace/sg2000_milkv_duos_musl_riscv64_sd.dtb"

if ! command -v dtc >/dev/null 2>&1; then
  echo "[decompile-dtb] dtc not found in PATH"
  exit 1
fi

if [ ! -f "$RUNTIME_DTB" ]; then
  echo "[decompile-dtb] missing runtime DTB: $RUNTIME_DTB"
  exit 1
fi

if [ ! -f "$GENERATED_DTB" ]; then
  echo "[decompile-dtb] missing generated DTB: $GENERATED_DTB"
  exit 1
fi

echo "[decompile-dtb] using dtc: $(command -v dtc)"
dtc -I dtb -O dts -o "$OUT_DIR/stock-runtime.dts" "$RUNTIME_DTB"
dtc -I dtb -O dts -o "$OUT_DIR/generated-boot.dts" "$GENERATED_DTB"
ls -lh "$OUT_DIR/stock-runtime.dts" "$OUT_DIR/generated-boot.dts"

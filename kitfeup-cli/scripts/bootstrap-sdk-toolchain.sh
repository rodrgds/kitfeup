#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="${ROOT_DIR:-$(pwd)}"
SDK_DIR="$ROOT_DIR/duo-buildroot-sdk-v2"
MUSL_CC="$SDK_DIR/host-tools/gcc/riscv64-linux-musl-x86_64/bin/riscv64-unknown-linux-musl-gcc"

if [ -x "$MUSL_CC" ]; then
  echo "[bootstrap-sdk-toolchain] SDK cross compiler already present"
  exit 0
fi

if [ ! -d "$SDK_DIR" ]; then
  echo "[bootstrap-sdk-toolchain] Missing SDK dir: $SDK_DIR" >&2
  exit 1
fi

if [ ! -x "$SDK_DIR/build.sh" ]; then
  echo "[bootstrap-sdk-toolchain] Missing executable: $SDK_DIR/build.sh" >&2
  exit 1
fi

echo "[bootstrap-sdk-toolchain] Cross compiler missing; running SDK bootstrap (./build.sh lunch)..."
cd "$SDK_DIR"
./build.sh lunch

if [ ! -x "$MUSL_CC" ]; then
  echo "[bootstrap-sdk-toolchain] SDK bootstrap finished but compiler is still missing: $MUSL_CC" >&2
  exit 1
fi

echo "[bootstrap-sdk-toolchain] SDK cross compiler is now available"
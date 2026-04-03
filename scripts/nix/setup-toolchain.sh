#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="${ROOT_DIR:-$(pwd)}"
SDK_DIR="$ROOT_DIR/duo-buildroot-sdk-v2"
HOST_TOOLS_DIR="$SDK_DIR/host-tools"
LIBNL_DIR="$ROOT_DIR/libnl-3.9.0"
SYSROOT_DIR="$ROOT_DIR/sysroot"
SYSROOT_LIB_DIR="$SYSROOT_DIR/lib"
SYSROOT_PKGCONFIG_DIR="$SYSROOT_LIB_DIR/pkgconfig"

if [ ! -d "$SDK_DIR" ]; then
  echo "[setup-toolchain] Missing SDK dir: $SDK_DIR" >&2
  exit 1
fi

echo "[setup-toolchain] Ensuring host-tools exists..."
if [ ! -d "$HOST_TOOLS_DIR/.git" ]; then
  git clone https://github.com/milkv-duo/host-tools.git "$HOST_TOOLS_DIR"
else
  echo "[setup-toolchain] host-tools already present"
fi

MUSL_CC="$HOST_TOOLS_DIR/gcc/riscv64-linux-musl-x86_64/bin/riscv64-unknown-linux-musl-gcc"
if [ ! -x "$MUSL_CC" ]; then
  echo "[setup-toolchain] Missing musl cross compiler: $MUSL_CC" >&2
  echo "[setup-toolchain] Run SDK board setup/build first (e.g. ./build.sh lunch) to populate toolchains." >&2
  exit 1
fi

if [ ! -f "$LIBNL_DIR/configure" ]; then
  echo "[setup-toolchain] Missing libnl source in $LIBNL_DIR" >&2
  echo "[setup-toolchain] Provide libnl-3.9.0 source folder before running this target." >&2
  exit 1
fi

echo "[setup-toolchain] Building static libnl into sysroot..."
mkdir -p "$SYSROOT_LIB_DIR" "$SYSROOT_PKGCONFIG_DIR"

cd "$LIBNL_DIR"
if [ ! -f "Makefile" ]; then
  ./configure \
    --host=riscv64-unknown-linux-musl \
    --disable-shared \
    --enable-static \
    --disable-cli \
    --prefix=/usr \
    CC="$MUSL_CC"
fi

make -j"$(nproc)"

cp -f lib/.libs/libnl-3.a "$SYSROOT_LIB_DIR/"
cp -f lib/.libs/libnl-genl-3.a "$SYSROOT_LIB_DIR/"

if [ -f libnl-3.0.pc ] && [ -f libnl-genl-3.0.pc ]; then
  cp -f libnl-3.0.pc "$SYSROOT_PKGCONFIG_DIR/"
  cp -f libnl-genl-3.0.pc "$SYSROOT_PKGCONFIG_DIR/"
else
  echo "[setup-toolchain] Warning: pkg-config files not found; build-libumdp may fail if pkg-config metadata is required." >&2
fi

echo "[setup-toolchain] Done"

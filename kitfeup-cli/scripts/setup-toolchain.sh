#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="${ROOT_DIR:-$(pwd)}"
SDK_DIR="$ROOT_DIR/duo-buildroot-sdk-v2"
HOST_TOOLS_DIR="$SDK_DIR/host-tools"
LIBNL_DIR="$ROOT_DIR/libnl-3.9.0"
SYSROOT_DIR="$ROOT_DIR/sysroot"
SYSROOT_USR_DIR="$SYSROOT_DIR/usr"
SYSROOT_LIB_DIR="$SYSROOT_USR_DIR/lib"
SYSROOT_PKGCONFIG_DIR="$SYSROOT_LIB_DIR/pkgconfig"
SYSROOT_INCLUDE_DIR="$SYSROOT_USR_DIR/include"
LIBNL_TARBALL_URL="https://github.com/thom311/libnl/releases/download/libnl3_9_0/libnl-3.9.0.tar.gz"

for cmd in git curl tar make; do
  if ! command -v "$cmd" >/dev/null 2>&1; then
    echo "[setup-toolchain] Missing required command: $cmd" >&2
    exit 1
  fi
done

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
MUSL_AR="$HOST_TOOLS_DIR/gcc/riscv64-linux-musl-x86_64/bin/riscv64-unknown-linux-musl-ar"
MUSL_STRIP="$HOST_TOOLS_DIR/gcc/riscv64-linux-musl-x86_64/bin/riscv64-unknown-linux-musl-strip"
if [ ! -x "$MUSL_CC" ]; then
  echo "[setup-toolchain] Missing musl cross compiler: $MUSL_CC" >&2
  echo "[setup-toolchain] Run SDK board setup/build first (e.g. ./build.sh lunch) to populate toolchains." >&2
  exit 1
fi
if [ ! -x "$MUSL_AR" ]; then
  echo "[setup-toolchain] Missing musl archiver: $MUSL_AR" >&2
  exit 1
fi
if [ ! -x "$MUSL_STRIP" ]; then
  echo "[setup-toolchain] Missing musl strip tool: $MUSL_STRIP" >&2
  exit 1
fi

if [ ! -f "$LIBNL_DIR/configure" ]; then
  echo "[setup-toolchain] libnl-3.9.0 source missing; downloading..."
  TMP_TARBALL="$ROOT_DIR/libnl-3.9.0.tar.gz"
  curl -fL "$LIBNL_TARBALL_URL" -o "$TMP_TARBALL"
  tar -xzf "$TMP_TARBALL" -C "$ROOT_DIR"
  rm -f "$TMP_TARBALL"
fi

if [ ! -f "$LIBNL_DIR/configure" ]; then
  echo "[setup-toolchain] Failed to prepare libnl source in $LIBNL_DIR" >&2
  exit 1
fi

echo "[setup-toolchain] Building static libnl into sysroot..."
mkdir -p "$SYSROOT_LIB_DIR" "$SYSROOT_PKGCONFIG_DIR" "$SYSROOT_INCLUDE_DIR"

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
ln -sfn usr/lib "$SYSROOT_DIR/lib"
rm -rf "$SYSROOT_INCLUDE_DIR/libnl3"
cp -a include/netlink "$SYSROOT_INCLUDE_DIR/"
mkdir -p "$SYSROOT_INCLUDE_DIR/libnl3"
cp -a include/netlink "$SYSROOT_INCLUDE_DIR/libnl3/"

if [ -f libnl-3.0.pc ] && [ -f libnl-genl-3.0.pc ]; then
  cp -f libnl-3.0.pc "$SYSROOT_PKGCONFIG_DIR/"
  cp -f libnl-genl-3.0.pc "$SYSROOT_PKGCONFIG_DIR/"
  sed -i "s|^prefix=.*$|prefix=$SYSROOT_USR_DIR|" "$SYSROOT_PKGCONFIG_DIR/libnl-3.0.pc"
  sed -i "s|^prefix=.*$|prefix=$SYSROOT_USR_DIR|" "$SYSROOT_PKGCONFIG_DIR/libnl-genl-3.0.pc"
else
  echo "[setup-toolchain] Warning: pkg-config files not found; build-libumdp may fail if pkg-config metadata is required." >&2
fi

for p in \
  "$SYSROOT_LIB_DIR/libnl-3.a" \
  "$SYSROOT_LIB_DIR/libnl-genl-3.a" \
  "$SYSROOT_INCLUDE_DIR/netlink/netlink.h" \
  "$SYSROOT_PKGCONFIG_DIR/libnl-3.0.pc" \
  "$SYSROOT_PKGCONFIG_DIR/libnl-genl-3.0.pc"; do
  if [ ! -e "$p" ]; then
    echo "[setup-toolchain] Missing expected output: $p" >&2
    exit 1
  fi
done

echo "[setup-toolchain] Done"

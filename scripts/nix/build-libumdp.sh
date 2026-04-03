#!/usr/bin/env bash
set -euo pipefail

CROSS_FILE="$ROOT_DIR/.cross-file.generated.txt"
trap 'rm -f "$CROSS_FILE"' EXIT

cat > "$CROSS_FILE" <<EOF
[binaries]
c = 'riscv64-unknown-linux-musl-gcc'
ar = 'riscv64-unknown-linux-musl-ar'
strip = 'riscv64-unknown-linux-musl-strip'
pkgconfig = 'pkg-config'

[properties]
pkg_config_libdir = '$ROOT_DIR/sysroot/lib/pkgconfig'

[host_machine]
system = 'linux'
cpu_family = 'riscv64'
cpu = 'riscv64'
endian = 'little'
EOF

meson setup "$ROOT_DIR/patched-umdp/libumdp/build" "$ROOT_DIR/patched-umdp/libumdp" \
  --cross-file "$CROSS_FILE" \
  --default-library=static \
  -Db_lto=false \
  --reconfigure

meson compile -C "$ROOT_DIR/patched-umdp/libumdp/build"

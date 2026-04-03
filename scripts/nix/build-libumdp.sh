#!/usr/bin/env bash
set -euo pipefail

CROSS_FILE="$ROOT_DIR/.cross-file.generated.txt"
PKGCONF_WRAPPER="$ROOT_DIR/.pkg-config-sysroot.sh"
trap 'rm -f "$CROSS_FILE" "$PKGCONF_WRAPPER"' EXIT

cat > "$PKGCONF_WRAPPER" <<EOF
#!/usr/bin/env bash
set -euo pipefail
unset PKG_CONFIG_SYSROOT_DIR
export PKG_CONFIG_LIBDIR='$ROOT_DIR/sysroot/usr/lib/pkgconfig'
exec pkg-config "\$@"
EOF
chmod +x "$PKGCONF_WRAPPER"

cat > "$CROSS_FILE" <<EOF
[binaries]
c = 'riscv64-unknown-linux-musl-gcc'
ar = 'riscv64-unknown-linux-musl-ar'
strip = 'riscv64-unknown-linux-musl-strip'
pkgconfig = '$PKGCONF_WRAPPER'

[properties]
pkg_config_libdir = '$ROOT_DIR/sysroot/usr/lib/pkgconfig'

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

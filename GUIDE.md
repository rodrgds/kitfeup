# KitFEUP Guide

Validated quick path from clean clone to working host build setup.

## 1) Host Requirements (NixOS)

- Use `nix-shell shell.nix` for all build steps.
- Fresh clone setup:

```sh
git clone https://github.com/rodrgds/kitfeup.git
cd kitfeup
git submodule update --init --recursive
make setup-toolchain
```

What `make setup-toolchain` does:
- clones `duo-buildroot-sdk-v2/host-tools` if missing,
- downloads `libnl-3.9.0` source if missing,
- builds static `libnl-3` and `libnl-genl-3`,
- installs libs/headers/pkg-config into `sysroot/usr/*`,
- verifies required outputs exist.

Notes:
- `duo-buildroot-sdk-v2/host-tools` is intentionally gitignored.
- `sysroot/` and `libnl-3.9.0/` are local build deps and must stay uncommitted.

## 2) Host Build Order

Run from repo root:

```sh
make build-libumdp
make build-umdp-ko
make build-shared
```

Or all at once:

```sh
make build-all
```

## 3) Sync to Board

```sh
make sync-compiled
make sync-umdp-ko
make sync-all
```

Reload module on board after syncing `umdp.ko`:

```sh
rmmod umdp 2>/dev/null || true
insmod shared/umdp.ko
```

## 4) Board Baseline

- Board: Milk-V Duo S (RISC-V mode)
- Known good image: `milkv-duos-musl-riscv64-sd_v2.0.1.img`

Flash once:

```sh
lsblk
sudo umount /dev/sdX?* 2>/dev/null || true
sudo dd if=milkv-duos-musl-riscv64-sd_v2.0.1.img of=/dev/sdX bs=4M conv=fsync status=progress
sync
```

First boot:

```sh
ssh root@192.168.42.1
# password: milkv
parted -s -a opt /dev/mmcblk0 "resizepart 3 100%"
resize2fs /dev/mmcblk0p3
mv /mnt/system/blink.sh /mnt/system/blink.sh_backup
sync
reboot
```

## 5) Boot Artifact Update (no full reflash)

```sh
make build-board-image
sudo mkdir -p /mnt/sdX1
sudo mount /dev/sdX1 /mnt/sdX1
sudo cp duo-buildroot-sdk-v2/install/soc_sg2000_milkv_duos_musl_riscv64_sd/boot.sd /mnt/sdX1/boot.sd
sudo umount /mnt/sdX1
sync
```

Do not `dd` `boot.sd` directly to partition raw blocks for iterative updates.

## 6) Demo Programs (on board)

```sh
shared/compiled/blink
shared/compiled/blink_umdp
shared/compiled/asm_led
shared/compiled/asm_add
shared/compiled/asm_fib
shared/compiled/uptime
shared/compiled/timer_wait
shared/compiled/blink_timer
shared/compiled/timer_multi
```

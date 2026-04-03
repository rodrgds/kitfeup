# KitFEUP Guide

This is the ordered setup for a **new host machine** and a **new Milk-V Duo S board**.

## 0) Prerequisites

- Host OS: NixOS
- Repo cloned and submodules initialized

```sh
git clone https://github.com/rodrgds/kitfeup.git
cd kitfeup
git submodule update --init --recursive
```

## 1) Host Setup (new machine)

From repo root:

```sh
make setup-toolchain
```

What this does:
- clones `duo-buildroot-sdk-v2/host-tools` if missing,
- downloads `libnl-3.9.0` if missing,
- builds static `libnl-3` and `libnl-genl-3`,
- populates `sysroot/usr/{lib,include}` and pkg-config files.

Notes:
- `duo-buildroot-sdk-v2/host-tools`, `sysroot/`, and `libnl-3.9.0/` are local dependencies and must not be committed.

## 2) Build Host Artifacts

Run all builds:

```sh
make build-all
```

Equivalent step-by-step:

```sh
make build-libumdp
make build-umdp-ko
make build-shared
```

## 3) Prepare SD Card (new Milk-V)

Known-good base image: `milkv-duos-musl-riscv64-sd_v2.0.1.img`

Flash once:

```sh
lsblk
sudo umount /dev/sdX?* 2>/dev/null || true
sudo dd if=milkv-duos-musl-riscv64-sd_v2.0.1.img of=/dev/sdX bs=4M conv=fsync status=progress
sync
```

## 4) First Boot Board Init (new Milk-V)

```sh
ssh root@192.168.42.1
# password: milkv
parted -s -a opt /dev/mmcblk0 "resizepart 3 100%"
resize2fs /dev/mmcblk0p3
mv /mnt/system/blink.sh /mnt/system/blink.sh_backup
sync
reboot
```

## 5) Sync Built Artifacts to Board

From host repo root:

```sh
make sync-all
```

Equivalent split sync:

```sh
make sync-compiled
make sync-umdp-ko
```

On board, reload module:

```sh
rmmod umdp 2>/dev/null || true
insmod shared/umdp.ko
```

## 6) Validate Demos on Board

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

## 7) Iterative Boot Artifact Updates (no full reflash)

When DT/kernel boot artifacts change:

```sh
make build-board-image
sudo mkdir -p /mnt/sdX1
sudo mount /dev/sdX1 /mnt/sdX1
sudo cp duo-buildroot-sdk-v2/install/soc_sg2000_milkv_duos_musl_riscv64_sd/boot.sd /mnt/sdX1/boot.sd
sudo umount /mnt/sdX1
sync
```

Do **not** `dd` `boot.sd` directly into partition raw blocks for iterative updates.

# KitFEUP Working Guide

This guide contains only the validated workflow currently used in the KitFEUP LCOM PoC.

## 1) Host Environment

- Host OS: NixOS
- Required workflow: run builds/sync from the project root using `nix-shell shell.nix`
- Before first build on a fresh clone, run `make setup-toolchain` (inside nix-shell)

Enter shell:

```sh
nix-shell shell.nix
make setup-toolchain
```

### Host Tools and libnl dependency policy

- `duo-buildroot-sdk-v2/host-tools` is intentionally gitignored (large external SDK dependency).
- `duo-buildroot-sdk-v2/build.sh` auto-clones `https://github.com/milkv-duo/host-tools.git` when `host-tools/` is missing.
- On NixOS, run host-tools/bootstrap steps inside `nix-shell shell.nix`.
- To pre-fetch host-tools via SDK flow, you can run:

```sh
cd duo-buildroot-sdk-v2
./build.sh <board>
# or
./build.sh lunch
```

- `sysroot/` and `libnl-3.9.0/` are treated as local build dependencies and should not be committed to git history.
- Repository setup target:

```sh
make setup-toolchain
```

This target ensures `host-tools/` is cloned (if missing) and builds static `libnl-3` + `libnl-genl-3` into `sysroot/lib` for the userspace and `libumdp` builds.

## 2) Board Baseline

Board: Milk-V Duo S in RISC-V mode.

Known-good baseline image:

- `milkv-duos-musl-riscv64-sd_v2.0.1.img`

Flash full image once:

```sh
lsblk
sudo umount /dev/sdX?* 2>/dev/null || true
sudo dd if=milkv-duos-musl-riscv64-sd_v2.0.1.img of=/dev/sdX bs=4M conv=fsync status=progress
sync
```

First boot board-side steps:

```sh
ssh root@192.168.42.1
# password: milkv
parted -s -a opt /dev/mmcblk0 "resizepart 3 100%"
resize2fs /dev/mmcblk0p3
mv /mnt/system/blink.sh /mnt/system/blink.sh_backup
sync
reboot
```

## 3) Build Flow (host)

All commands below should be run from repository root (inside nix-shell).

Build everything:

```sh
make build-all
```

Build specific parts:

```sh
make build-libumdp
make build-umdp-ko
make build-shared
```

## 4) Sync Flow (host -> board)

```sh
make sync-compiled
make sync-umdp-ko
make sync-all
```

Reload kernel module on board after syncing `umdp.ko`:

```sh
rmmod umdp 2>/dev/null || true
insmod shared/umdp.ko
```

## 5) Boot Artifact Update Flow (validated)

To update DT/kernel boot artifacts without reflashing full image:

1. Build SDK boot artifacts:

```sh
make build-board-image
```

2. Copy `boot.sd` to SD partition 1 (VFAT):

```sh
sudo mkdir -p /mnt/sdX1
sudo mount /dev/sdX1 /mnt/sdX1
sudo cp duo-buildroot-sdk-v2/install/soc_sg2000_milkv_duos_musl_riscv64_sd/boot.sd /mnt/sdX1/boot.sd
sudo umount /mnt/sdX1
sync
```

Important:

- Do not `dd` a standalone `boot.sd` into partition raw blocks for iterative updates.
- Mount and copy into filesystem as above.

## 6) Demo Programs

Run on board from `/root` (after sync):

```sh
shared/compiled/blink
shared/compiled/blink_umdp
shared/compiled/asm_led
shared/compiled/uptime
shared/compiled/timer_wait
shared/compiled/blink_timer
shared/compiled/timer_multi
```

`timer_multi` demonstrates multiple logical software timers (e.g. 250 ms and 1100 ms) on top of one hardware periodic IRQ tick.

## 7) Current Technical State

- Active UMDP implementation is `patched-umdp/`.
- UMDP interrupt path is generic (subscribe/unsubscribe/unmask + delivery).
- Timer policy (period handling, duplicate filtering, software multi-timers) lives in userspace timer libraries under `shared/src/timer/`.
- SDK DTS patches for DW timer interrupt wiring are applied in `duo-buildroot-sdk-v2`.

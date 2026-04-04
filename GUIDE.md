# KitFEUP Guide

This guide is now centered around the `kitfeup` CLI.

Two CLI binaries are built from the same source:
- **`kitfeup`**: host CLI (runs on your Linux machine)
- **`kitfeupb`**: board CLI (runs on the Milk-V Duo S)

## 0) Host environment setup

Required:
- Linux host
- Git
- Nix (`nix-shell`)

Install Nix (if missing):

```sh
sh <(curl -L https://nixos.org/nix/install) --daemon
```

Then restart your shell/session.

Optional board connection overrides (defaults shown):

```sh
export BOARD_HOST=192.168.42.1
export BOARD_USER=root
export BOARD_PASS=milkv
export BOARD_SHARED_DIR=/root/shared
```

## 1) Clone

```sh
git clone https://github.com/rodrgds/kitfeup.git
cd kitfeup
git submodule update --init --recursive
```

## 2) Build CLI binaries

```sh
NO_SYNC=1 make kitfeup-cli
```

This builds:
- host CLI: `kitfeup-cli/bin/kitfeup`
- board CLI: `kitfeup-cli/bin/kitfeupb`

`NO_SYNC=1` is required for first setup because the board may not be ready/reachable yet.

Then run host setup and host doctor:

```sh
./kitfeup setup
./kitfeup doctor
```

## 3) Prepare SD card

Download and flash the base image first:

```sh
./kitfeup image download-latest
./kitfeup image flash /dev/sdX [path/to/image.img|.img.xz|.img.zip]
```

Then build and install custom `boot.sd`:

```sh
./kitfeup image build-boot
./kitfeup image install-boot /dev/sdX1
```

## 4) First board boot init

Insert the SD card, power on the board, and connect to it via SSH:

```sh
ssh root@192.168.42.1
# password: milkv
```

Sync the `kitfeupb` CLI to the board:

```sh
./kitfeup sync
```

On board (you can run `source /root/.profile` to get `kitfeupb` in your PATH without rebooting):

```sh
kitfeupb setup
reboot
```

## 5) Build + sync project artifacts

From host repo root:

```sh
./kitfeup
```

Default host flow:
- setup if needed
- build (`libumdp`, `umdp.ko`, `shared/compiled`)
- sync to board (including `kitfeupb`)

Useful variants:

```sh
./kitfeup compile --no-sync
./kitfeup sync
./kitfeup doctor
```

## 6) Validate on board

```sh
kitfeupb doctor
shared/compiled/timer_wait
shared/compiled/blink_timer
shared/compiled/timer_multi
```

If UMDP module is not loaded:

```sh
insmod /root/shared/umdp.ko
```

## 7) Iterative boot updates (no full reflash)

When kernel/DT changes and you only need updated `boot.sd`:

```sh
./kitfeup image build-boot
./kitfeup image install-boot /dev/sdX1
```

Do not write `boot.sd` to raw disk offsets.
`boot.sd` must be copied into the mounted boot partition filesystem.

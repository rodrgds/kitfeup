# KitFEUP - Milk-V Duo S PoC

<img src="logo.png" alt="KitFEUP logo" width="180" />

KitFEUP is a proof-of-concept for using the Milk-V Duo S (SG2000, RISC-V) as a low-cost teaching platform, currently focused on the LCOM user-space driver workflow with UMDP, but with potential for broader embedded/RISC-V education along the whole degree (OS, compilers, architecture, distributed systems, etc).

The repository now uses a CLI-first workflow:
- **`kitfeup`**: host CLI (runs on your Linux machine)
- **`kitfeupb`**: board CLI (runs on the Milk-V Duo S, synced via `./kitfeup sync`)

## Quick Start

```sh
git clone https://github.com/rodrgds/kitfeup.git
cd kitfeup
git submodule update --init --recursive
make kitfeup-cli
./kitfeup
```

For complete first-time SD setup and board initialization, see [GUIDE.md](GUIDE.md).

## Main Commands

### Host

```sh
./kitfeup
./kitfeup doctor
./kitfeup compile --no-sync
./kitfeup sync
./kitfeup image build-boot
./kitfeup image download-latest
./kitfeup image flash /dev/sdX
./kitfeup image install-boot /dev/sdX1
```

### Board

```sh
kitfeupb doctor
kitfeupb setup
kitfeupb run timer_wait
```

## Project Layout

- `kitfeup-cli/` - host and board CLI sources/binaries
- `shared/` - userspace demos and timer/LED libraries
- `patched-umdp/` - active UMDP codebase
- `duo-buildroot-sdk-v2/` - SDK + kernel/boot build tree
- `kitfeup-cli/scripts/` - build/sync/image helper scripts used by Make/CLI

## Notes

- Host builds require Linux + Nix (`nix-shell`).
- Default board connection expects `root@192.168.42.1` with password `milkv`.
- Board timer demos require custom `boot.sd` from this repo's SDK build flow.

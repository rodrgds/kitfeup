# kitfeup-cli

Standalone CLI program for KitFEUP automation.

## Build

Host binary (`kitfeup`):

```sh
make -C kitfeup-cli host
```

RISC-V binary (`kitfeupb`, for board mode):

```sh
make -C kitfeup-cli board
```

Or from repo root:

```sh
make kitfeup-cli
```

## Run

```sh
./kitfeup-cli/bin/kitfeup help
./kitfeup-cli/bin/kitfeup doctor
./kitfeup-cli/bin/kitfeup setup
./kitfeup-cli/bin/kitfeup compile
./kitfeup-cli/bin/kitfeup sync
./kitfeup-cli/bin/kitfeup image build
./kitfeup-cli/bin/kitfeup image download-latest
./kitfeup-cli/bin/kitfeup image flash /dev/sdX [image]

# from repo root (wrapper)
./kitfeup help

# build and sync board binary
./kitfeup sync
```

After sync, the board PATH is automatically updated to include `/root/shared`,
so you can run `kitfeupb` directly after opening a new shell/session.

Default with no arguments runs setup + compile + sync.

Board-side (`kitfeupb`) first boot setup:

```sh
kitfeupb setup
```

This applies GUIDE step 5 style actions:
- resize partition 3 with `parted`
- run `resize2fs /dev/mmcblk0p3`
- disable `/mnt/system/blink.sh` by renaming it to `blink.sh_backup`

## Extensibility for courses

External executable plugins are supported and split by mode:

- Host mode:
  - `kitfeup.d/host/<name>`
  - `~/.config/kitfeup/plugins/host/<name>`

- Board mode:
  - `kitfeup.d/board/<name>`
  - `~/.config/kitfeup/plugins/board/<name>`

If present and executable, they can be called as:

```sh
kitfeup <name> ...
```

Example idea:

```sh
kitfeup lcom run lab3
```

This keeps host and board commands clearly separated while letting teachers ship course-specific commands.

## Image workflows

- `kitfeup image build` -> builds board image artifacts
- `kitfeup image strip-cimg <boot.sd>` -> strips CIMG header in place
- `kitfeup image download-latest [owner/repo]` -> downloads latest release assets to `downloads/` (`.img`, `.img.xz`, `.img.zip`, boot files)
- `kitfeup image flash <device> [image]` -> flashes image to SD card device (`.img`, `.img.xz`, `.img.zip`)
  - shows target drive size in GB and asks `Proceed with flash? [y/N]` (default no)

## Binary names

- Host: `kitfeup`
- Board: `kitfeupb`

Build both from repo root (and sync `kitfeupb` by default):

```sh
make kitfeup-cli
```

Build without syncing:

```sh
make kitfeup-cli NO_SYNC=1
./kitfeup compile --no-sync
```

This generates:

- `kitfeup-cli/bin/kitfeup`
- `kitfeup-cli/bin/kitfeupb`

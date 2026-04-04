# KitFEUP PoC Change Log (curated)

This log keeps only high-signal milestones that are relevant to the current working setup.

## Milestone 1 - Nix-first host environment

- Added `shell.nix` with required host tools for cross-build, packaging, and sync.
- Standardized build/sync entrypoints via root `Makefile` and `kitfeup-cli/scripts/*`.

## Milestone 2 - UMDP operational on Milk-V Duo S

- Adopted `patched-umdp/` as active UMDP codebase for Linux 5.10 compatibility.
- Built and deployed `umdp.ko` plus `libumdp` userspace library.
- Restored working userspace `mmap` and connection flow for demos.

## Milestone 3 - GPIO and baseline demos

- Validated LED control through:
  - sysfs path (`blink`)
  - UMDP MMIO path (`blink_umdp`)
  - assembly-assisted UMDP path (`asm_led`)

## Milestone 4 - Timer interrupt path

- Added SG2000 DW timer DTS wiring in SDK:
  - `duo-buildroot-sdk-v2/build/boards/default/dts/cv181x/cv181x_base.dtsi`
  - `duo-buildroot-sdk-v2/build/boards/default/dts/cv181x_riscv/cv181x_base_riscv.dtsi`
- Added IRQ subscribe/unsubscribe/unmask flow across UMDP kernel/userspace interfaces.
- Stabilized userspace timer behavior (`timer_wait`, `blink_timer`) against duplicate/late notifications.

## Milestone 5 - Generic UMDP IRQ path + userspace timer policy

- Removed timer-specific IRQ mapping behavior from UMDP core.
- Kept UMDP as generic interrupt transport/control component.
- Moved timer policy fully to userspace timer library (`shared/src/timer/timer_irq.*`).

## Milestone 6 - Multi logical timers

- Added `timer_multi` library:
  - `shared/src/timer/timer_multi.h`
  - `shared/src/timer/timer_multi.c`
- Added demo program:
  - `shared/src/timer_multi.c`
- Demonstrates multiple software timers (e.g. 250 ms and 1100 ms) over one hardware periodic IRQ tick.

## Milestone 7 - Repository hygiene

- Removed unused upstream UMDP copy (`linux-usermode-driver-platform`).
- Untracked generated binaries from `shared/compiled`.
- Expanded root `.gitignore` for local build artifacts, images, and temporary files.
- Flattened assembly demo sources from `shared/src/assembly/` to `shared/src/` for consistency with program-centric layout.

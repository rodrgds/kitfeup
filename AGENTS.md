# Project Context & AI Agent Directives

## Project Overview: KitFEUP
This repository contains the workspace for **KitFEUP**, a proof-of-concept initiative aimed at overhauling the L.EIC (Licenciatura em Engenharia Informática e Computação) degree curriculum at FEUP.

The ultimate goal is to provide every student with a standardized, low-cost RISC-V Single Board Computer to be used continuously across multiple disciplines:
* **LCOM (Laboratório de Computadores):** Developing low-level device drivers and handling interrupts in user space.
* **Computer Architecture:** Writing and executing RISC-V assembly natively (bare-metal or Linux-backed) to interact with hardware.
* **Compilers:** Serving as the physical target architecture for student-built compilers.
* **Distributed Systems:** Acting as physical nodes for decentralized networking concepts.

The immediate active phase is completing the **LCOM proof-of-concept** using the User Mode Driver Platform (UMDP) to prove hardware viability to the faculty.

## Hardware Target
* **Board:** Milk-V Duo S (Model MV003-W1E8P1)
* **SoC:** Sophgo SG2000 (RISC-V Architecture)
* **Relevant Peripherals:** Onboard User LED, Hardware Timers, RTC.
* **Debugging:** 3.3V USB-to-TTL UART adapter (necessary to recover from kernel panics over serial console).

## Host Environment & Build Rules (CRITICAL)
* **Host OS:** NixOS.
* **Rule 1:** Standard pre-compiled C toolchains downloaded by SDKs will fail instantly due to missing `/lib64` FHS paths.
* **Rule 2:** All SDK build scripts, `make` commands, and cross-compilations MUST be executed inside the NixOS FHS sandbox. Enter this environment by running `nix-shell shell.nix` in the project root.

## Directory Structure Breakdown
* `/duo-buildroot-sdk-v2`: Official SDK. Contains the `linux_5.10` kernel tree required to compile the UMDP module against the correct SG2000 kernel headers.
* `/sg2000`: Hardware datasheets.
    * `SG2000_TRM_V1.0-alpha.pdf`: Technical Reference Manual. Use this to locate exact base hexadecimal Memory-Mapped I/O (MMIO) addresses for GPIO and Timers.
    * `duo_s_SCH_v1.1.pdf`: Board schematic. Use this to map the physical User LED to its specific SoC GPIO pin name.
* `/patched-umdp`: Active UMDP source code, cross-compilation shell scripts, and the user-space library required for memory mapping physical addresses via `umdp_mmap_physical()`.
* `/PI`: Legacy documentation, reference code, and the guide for flashing Linux to the board.
* `/milkv.io`: Local clone of the official documentation website.
* `shell.nix`: The NixOS FHS environment configuration.

## Current LCOM Proof-of-Concept Milestones
1.  Flash Buildroot Linux to the Milk-V Duo S.
2.  Blink the onboard LED using standard `/sys/class/gpio` files and `sleep()`.
3.  Cross-compile and load the UMDP kernel module into the Milk-V.
4.  Rewrite the LED blinker using UMDP's `umdp_mmap_physical()` API to directly manipulate memory registers.
5.  Implement a user-space device driver to configure an SG2000 hardware timer to trigger periodic interrupts.
6.  Handle the hardware timer interrupt via UMDP to blink the LED without using software delays.

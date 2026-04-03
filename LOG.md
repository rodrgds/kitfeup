- cloned all relevant repositories to this repo itself
- created `shell.nix` with a NixOS FHS environment and added the SDK toolchain to the PATH so that we can use the `riscv64-unknown-linux-musl-gcc` compiler directly from the terminal (using the `compile` alias for convenience)
---
- get the latest linux image from https://github.com/milkv-duo/duo-buildroot-sdk-v2/releases (sd version) - this one: `milkv-duos-musl-riscv64-sd_v2.0.1.img.zip`
  - for the Duo S
  - Risc-V 64-bit
  - SD card version
  - we could also look at other distros here: https://milkv.io/docs/duo/getting-started/download
- do an `lsblk` figure out what the MicroSD card is (e.g. /dev/sdb)
  - the disk, not partitions (e.g. /dev/sdb, not /dev/sdb1)
- flash the image with dd (or balenaEtcher or Rufus if you prefer a GUI)
  - `sudo dd if=milkv-duos-musl-riscv64-sd_v2.0.1.img of=/dev/sdc bs=4M status=progress`
- finally, run `sync` to ensure all data is written to the SD card
---
- insert the SD card into the Duo S
- switch to RV (physically)
- connect the USB-A to USB-C cable to the Duo S and your computer
- after plugged in, in about 5 seconds the LED should flash blue - that means the kernel successfully booted!
- the milk-v assigns itself a static ip: `192.168.42.1`
- connect to the milk-v via ssh: `ssh root@192.168.42.1` with the password `milkv`
- aaaaand we're in!
- to finish, we should expand the root partition to use the full SD card space, since by default it only uses a small portion of it
  - `parted -s -a opt /dev/mmcblk0 "resizepart 3 100%"`
  - and expand the filesystem: `resize2fs /dev/mmcblk0p3`
  - to verify, run `df -h`
---
- the LED is still blinking blue. We want to stop it so that we can control it ourselves!
  - `mv /mnt/system/blink.sh /mnt/system/blink.sh_backup` moves it out of the way!
  - then `sync` and `reboot` to restart the system without the blinking script
- we can look at what the script is actually doing `cat /mnt/system/blink.sh_backup`

```c
#!/bin/sh

LED_PIN=509

LED_GPIO=/sys/class/gpio/gpio${LED_PIN}

if test -d ${LED_GPIO}; then
    echo "PIN ${LED_PIN} already exported"
else
    echo ${LED_PIN} > /sys/class/gpio/export
fi

echo out > ${LED_GPIO}/direction

while true; do
    echo 0 > ${LED_GPIO}/value
    sleep 0.5
    echo 1 > ${LED_GPIO}/value
    sleep 0.5
done
```

- analyzed this and mimicked in the `shared/blink.c` file
- cloned `duo-buildroot-sdk-v2` repo as well as `host-tools` into there to get access to the toolchain + compiler
- created `sync.sh` and `connect.sh` scripts to automate the process of syncing files to the board and connecting via ssh
- now I can run `compile shared/blink.c -o shared/blink` to compile the C code with the SDK toolchain, then `./sync.sh` to sync it to the board, and finally `./connect.sh` to ssh into the board and run `./shared/blink` to see the LED blinking again, but this time controlled by our own program which we can `Ctrl+C` to stop whenever we want.
- created the `cross-file.txt` to tell Meson how to use our RISC-V toolchain for cross-compilation, and added the necessary build dependencies (Meson and Ninja) to the Nix shell as well as flex and bison for parsing.
- added `.gitignore` to exclude clangd cache (`.cache/`) and build artifacts

```sh
# Download and extract libnl
wget https://github.com/thom311/libnl/releases/download/libnl3_9_0/libnl-3.9.0.tar.gz
tar -xzf libnl-3.9.0.tar.gz
cd libnl-3.9.0

# Configure it for RISC-V and force it to be static
./configure --host=riscv64-unknown-linux-musl --prefix=/home/rgo/uni/kitfeup/sysroot --enable-static --disable-shared

# Compile and install to the sysroot folder
make -j$(nproc)
make install
```

Then, to build the UMDP library with Meson:

```sh
cd patched-umdp/libumdp
meson setup build --cross-file ../../cross-file.txt --default-library=static -Db_lto=false
meson compile -C build
```

Now we need to build the actual kernel module (umdp.ko). I had to add `openssl` and `openssl.dev` to the Nix shell dependencies to get the necessary headers for the kernel module to compile successfully.

```sh
cd duo-buildroot-sdk-v2

# Source the Milk-V build environment
source build/envsetup_milkv.sh
# Choose 7. milkv-duos-musl-riscv64-sd

# Now we can use the `build_kernel` function defined in the environment to build the kernel module
build_kernel
```

Now we can actually build the kernel module!

```sh
cd ~/uni/kitfeup/patched-umdp/umdp

make -C ../../duo-buildroot-sdk-v2/linux_5.10/build/sg2000_milkv_duos_musl_riscv64_sd M=$PWD ARCH=riscv CROSS_COMPILE=riscv64-unknown-linux-musl- modules
```

Using the original upstream UMDP gave us issues because it targets Linux 6.x, while the Milk-V is on Linux 5.10. The patched UMDP tree includes the compatibility fixes we need for this kernel.

After those fixes, we now have a good `umdp.ko` kernel module!

```sh
# copy to our shared folder
cp umdp.ko ~/uni/kitfeup/shared/

# sync to the board
cd ~/uni/kitfeup
./sync.sh

# connect to the board
./connect.sh

# on the board, we can now load the kernel module
insmod shared/umdp.ko
```

## Debugging Kernel Module Issues

When the UMDP connection fails with "Unspecific failure", check dmesg on the board:

```sh
dmesg | grep umdp
```

### Issue 1: Socket Validation (portid 0)
Key issue found: The `partial_netlink_sock` struct alignment is wrong for Linux 5.10, causing the kernel to read portid 0 instead of the actual portid. This breaks the socket validation check.

**Symptom:** `dmesg` shows `found netlink socket with portid 0:`

**Fix:** Bypass the broken socket validation in `umdp_core.c`. After applying the bypass, dmesg should show:
`umdp: BYPASSING broken socket validation (would look for portid X)`

### Issue 2: mmap Access Control
After fixing the socket validation, mmap fails with "Operation not permitted".

**Symptom:** `umdp_mmap_physical: mmap failed: Operation not permitted`

**Fix:** Bypass the mmap access control check in `umdp_mem_mmap()`. The access control system checks if the process's executable is in a permission table, which isn't configured for our programs.

### Recompiling and Reloading
After applying fixes, recompile and reload:
```sh
# On host (in nix-shell)
cd patched-umdp/umdp
make -C ../../duo-buildroot-sdk-v2/linux_5.10/build/sg2000_milkv_duos_musl_riscv64_sd \
     M=$PWD ARCH=riscv CROSS_COMPILE=riscv64-unknown-linux-musl- modules
cp umdp.ko ../../shared/
cd ../..
./sync.sh

# On board
rmmod umdp
insmod shared/umdp.ko
dmesg | grep umdp
```

## Step 4: LED Blink using UMDP with sleep()

Created `shared/blink_umdp.c` - a program that uses UMDP's `umdp_mmap_physical()` to directly access GPIO registers instead of using `/sys/class/gpio` files.

**Hardware details:**
- Milk-V Duo S onboard LED is on GPIO pin 509 (GPIO0/GPIOA bank, pin 29; sysfs #509 = 480 + 29)
- GPIO0 (GPIOA) base address: 0x03020000
- GPIO registers: Data (0x00), Direction (0x04)

**How it works:**
1. Connects to UMDP
2. Maps GPIO registers using `umdp_mmap_physical()`
3. Sets GPIO pin as output
4. Toggles the pin in a loop with `usleep()` delays

## Step 5: Timer Interrupt Test

Created `shared/timer_wait.c` - a program that uses the timer library to wait for
hardware timer interrupts via UMDP.

**How it works:**
1. Connects to UMDP
2. Initializes timer using the timer library (`timer_irq_init`)
3. Configures timer for periodic interrupts with configurable period
4. Waits for timer interrupts using `timer_irq_wait_tick()`
5. Accepts command line argument for period in milliseconds

**IRQ note:** switched UMDP from hardcoded Linux IRQ assumptions/poll fallback to IRQ-domain resolution (requested hwirq -> Linux IRQ mapping via PLIC).

## Timer IRQ Wiring Fix

To move toward real interrupts, added timer DT declarations so Linux can describe the DW APB timer at `0x030A0000` with explicit PLIC interrupt wiring.

Changed files:
- `duo-buildroot-sdk-v2/build/boards/default/dts/cv181x/cv181x_base.dtsi`
- `duo-buildroot-sdk-v2/build/boards/default/dts/cv181x_riscv/cv181x_base_riscv.dtsi`

Changed UMDP behavior:
- Removed timer-specific fallback polling path in `umdp_core.c`.
- Added IRQ-domain resolution (`riscv,plic0`) before `request_irq`, so requested hwirq values are mapped to Linux IRQ numbers explicitly.

## Board image packaging workflow

- Added SDK DTS changes for timer IRQ wiring:
  - `duo-buildroot-sdk-v2/build/boards/default/dts/cv181x/cv181x_base.dtsi`
  - `duo-buildroot-sdk-v2/build/boards/default/dts/cv181x_riscv/cv181x_base_riscv.dtsi`
- Added a reusable board image build script:
  - `scripts/nix/build-board-image.sh`
- Added Makefile target:
  - `make build-board-image`
- This target runs SDK boot artifact packaging (`build_uboot` + `build_kernel`) and prints generated `boot.sd`/`boot.itb`/`fip` artifacts so flashing is done manually.
- Current environment note: full Buildroot `sd_image` (`.img`) packaging may fail when rootfs artifacts are missing; `boot.sd` generation is working and includes the updated DT.

## Timer IRQ + libnl fixes (latest)

- `timer_wait` was receiving duplicated IRQ notifications because the libumdp callback/parser path was partially bypassed and IRQ queue drain behavior was missing.
- `blink_timer` could block forever waiting for an IRQ when no new interrupt arrived after the first event.

What changed in `patched-umdp`:

- `libumdp/src/protocol-family.c`:
  - registered `UMDP_CMD_INTERRUPT_UNMASK` in the generic netlink command table.
- `libumdp/include/umdp.h`:
  - added `umdp_interrupt_unmask()` API.
  - added `umdp_receive_interrupt_timeout()` and `umdp_receive_interrupt_nowait()` APIs.
- `libumdp/src/umdp.c`:
  - restored proper libnl callback path (`NL_CB_VALID -> genl_handle_msg`).
  - restored connect command reply handling instead of blind success.
  - kept PID/port setup and relaxed seq/message checks needed for current environment.
  - implemented timeout and nowait interrupt receive helpers.
  - kept `UMDP_CMD_INTERRUPT_UNMASK` command support on userspace side.

What changed in `shared` programs/libraries:

- `shared/src/timer/timer_irq.c`:
  - switched tick wait to timeout-based receive.
  - on timeout: clear status/EOI and unmask IRQ, then return `ETIMEDOUT` so callers can report it.
  - after successful tick: drain any immediately queued duplicate IRQ notifications with non-blocking receive before returning.
- `shared/src/timer_wait.c`:
  - handles `ETIMEDOUT` with a warning and continues running.
- `shared/src/blink_timer.c`:
  - handles `ETIMEDOUT` with a warning and continues running.

Current reliable board flashing flow:

- Flash full stock image to `/dev/sdX` with `dd` only for full reflash.
- For kernel/DT iterative updates, mount `/dev/sdX1` and copy generated `boot.sd` to `/mnt/sdX1/boot.sd`.
- Do not patch `boot.sd` into raw image offsets for iterative work.

## Step 5b: PWM Output Test via UMDP

Started a UMDP PWM prototype (`pwm_test.c` and `pwm_umdp.{h,c}`) and moved it to
`shared/src/others/` for future work. It is intentionally not built by default.

## Additional Hardware Drivers

### Read RTC Counter
`shared/uptime.c` - Displays the RTC seconds counter.
**Note:** On this image, the RTC block is not enabled at boot, so the program enables it with `rtc_ensure_running(..., 0)`. This avoids setting a fake date; the shown value is seconds since counter enable (often first run after power cycle).

### Breathing LED (Software PWM)
`shared/breathing_led.c` - Smooth LED fade using software PWM.
**Note:** The onboard LED (GPIOA[29]) does NOT have a hardware PWM alternate function on SG2000. Uses fast GPIO toggling instead.

**Hardware:** GPIO0 Base 0x03020000, Pin 29

**Usage:** `./shared/breathing_led`

### SARADC - NOT ACCESSIBLE
The SG2000 SARADC clock is gated and has no driver interface on this kernel build.
- Raw memory access via UMDP causes segfault (no clock)
- No IIO subsystem compiled into kernel
- No sysfs files for ADC values
- `/dev/cvitekaadc` is the audio codec ADC, not the SARADC

For analog input, you would need to recompile the kernel with IIO/SARADC drivers enabled.

**Hardware:** Base 0x030F0000, channel 6 or 7 for temperature sensor

**Usage:**
```sh
./shared/thermometer_driver      # Default channel 6
./shared/thermometer_driver 7    # Use channel 7
```

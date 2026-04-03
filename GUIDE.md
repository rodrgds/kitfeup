# KitFEUP - Complete Setup Guide

This guide walks through setting up the Milk-V Duo S for the LCOM proof-of-concept using UMDP.

## 1. Prerequisites

### Hardware
- Milk-V Duo S (MV003-W1E8P1)
- MicroSD card (8GB+)
- USB to MicroSD card reader (to flash the image)
- USB-A to USB-C cable (to power and connect to the board)
- 3.3V USB-to-TTL UART adapter (for debugging/recovery)

### Software
- Linux host (tested on NixOS)
- Nix package manager with FHS environment support

## 2. Flash Linux to Milk-V Duo S

### Download Image
```sh
wget https://github.com/milkv-duo/duo-buildroot-sdk-v2/releases/download/v2.0.1/milkv-duos-musl-riscv64-sd_v2.0.1.img.zip
unzip milkv-duos-musl-riscv64-sd_v2.0.1.img.zip
```

If you already have the zip locally, skip `wget` and run `unzip` in that directory.

### Flash to SD Card
```sh
lsblk
sudo umount /dev/sdc?* 2>/dev/null || true
sudo dd if=milkv-duos-musl-riscv64-sd_v2.0.1.img of=/dev/sdc bs=4M conv=fsync status=progress
sync
```

Replace `/dev/sdc` with your SD device from `lsblk` if different.

### Clean End-to-End Reflash + Timer-Boot Update (Recommended)

Use this when you want to start clean from the stock image, then apply the timer-interrupt boot update.

```sh
# 0) Set paths (works from any current directory)
IMG_DIR="$PWD"
IMG_ZIP="$IMG_DIR/milkv-duos-musl-riscv64-sd_v2.0.1.img.zip"
IMG_FILE="$IMG_DIR/milkv-duos-musl-riscv64-sd_v2.0.1.img"
REPO_DIR="/home/rgo/uni/kitfeup"
BOOT_SD="$REPO_DIR/duo-buildroot-sdk-v2/install/soc_sg2000_milkv_duos_musl_riscv64_sd/boot.sd"

# 1) Unzip stock image (skip if already unzipped)
unzip -o "$IMG_ZIP"

# 2) Flash stock full image
sudo umount /dev/sdc?* 2>/dev/null || true
sudo dd if="$IMG_FILE" of=/dev/sdc bs=4M conv=fsync status=progress
sync

# 3) Boot board once with stock image and SSH in (password: milkv)
# ssh root@192.168.42.1

# 4) On board, expand root partition, disable auto-blink, then reboot:
# parted -s -a opt /dev/mmcblk0 "resizepart 3 100%"
# resize2fs /dev/mmcblk0p3
# mv /mnt/system/blink.sh /mnt/system/blink.sh_backup
# sync
# reboot

# 5) Power off board, reinsert SD in host, then update only boot partition with timer-fixed boot.sd
# Mount the boot partition first!
sudo mkdir -p /mnt/sdc1
sudo mount /dev/sdc1 /mnt/sdc1
sudo cp "$BOOT_SD" /mnt/sdc1/boot.sd
sudo umount /mnt/sdc1
sync
```

After step 5, reinsert SD in the board and boot again.
If `192.168.42.1` is not reachable, check host interfaces with `ip -br addr` and use the detected board IP.

### Important: boot update method that actually works

Only this method is reliable for timer/DT updates:

1. Flash stock full image with `dd` to `/dev/sdX`.
2. Build updated artifacts in SDK.
3. Mount partition 1 (`/dev/sdX1`) and copy `boot.sd` into the mounted filesystem.

Do not try to inject `boot.sd` into a raw `.img` file with byte offsets for iterative updates. It is error-prone and was not reliable in our tests.

### First Boot
1. Insert SD card into Milk-V Duo S
2. Switch to RV mode (physical switch)
3. Connect USB cable to computer
4. Wait ~5 seconds for blue LED to flash (kernel booted)
5. Connect via SSH:
   ```sh
   ssh root@192.168.42.1
   # Password: milkv
   ```

### Expand Root Partition
```sh
parted -s -a opt /dev/mmcblk0 "resizepart 3 100%"
resize2fs /dev/mmcblk0p3
df -h
```

### Disable Auto-Blink
```sh
mv /mnt/system/blink.sh /mnt/system/blink.sh_backup
sync
reboot
```

## 3. Setup Development Environment

### Clone Repository
```sh
git clone <repo-url> kitfeup
cd kitfeup
```

### Enter Nix Shell
```sh
nix-shell
# Provides RISC-V cross-compiler, meson, sshpass, and build tools
```

### Available Shell Commands
- `compile` - Simple wrapper for `riscv64-unknown-linux-musl-gcc`
- `my-compile` - Compile with UMDP library paths included

## 4. Build UMDP Library

### Build libnl (Dependency)
```sh
wget https://github.com/thom311/libnl/releases/download/libnl3_9_0/libnl-3.9.0.tar.gz
tar -xzf libnl-3.9.0.tar.gz
cd libnl-3.9.0

./configure --host=riscv64-unknown-linux-musl \
            --prefix=/home/rgo/uni/kitfeup/sysroot \
            --enable-static --disable-shared

make -j$(nproc)
make install
cd ..
```

### Build UMDP Library
```sh
cd patched-umdp/libumdp
meson setup build --cross-file ../../cross-file.txt --default-library=static -Db_lto=false
meson compile -C build
cd ../..
```

## 5. Build Kernel Module

### Setup Kernel Build Environment
```sh
cd duo-buildroot-sdk-v2
source build/envsetup_milkv.sh
# Choose option 7: milkv-duos-musl-riscv64-sd
build_kernel
cd ..
```

### Compile UMDP Kernel Module
```sh
cd patched-umdp/umdp
make -C ../../duo-buildroot-sdk-v2/linux_5.10/build/sg2000_milkv_duos_musl_riscv64_sd \
     M=$PWD ARCH=riscv CROSS_COMPILE=riscv64-unknown-linux-musl- modules
cp umdp.ko ../../shared/
cd ../..
```

## 6. Build and Deploy with Makefile (Recommended)

All Makefile targets run inside `nix-shell shell.nix` automatically.

### Build Everything
```sh
make build-all
```

### Build Board Image Artifacts (for reflashing)
```sh
make build-board-image
```

This runs the SDK flow (`envsetup_milkv.sh` + `build_uboot` + `build_kernel`), prints produced boot artifacts, and strips the incompatible CIMG header from `boot.sd` so it is a standard FIT image.

### Flash Artifacts (`/dev/sdc` example)
```sh
# 1) Reflash full card image (if you have a full .img)
sudo umount /dev/sdc?* 2>/dev/null || true
sudo dd if=/path/to/image.img of=/dev/sdc bs=4M conv=fsync status=progress
sync

# 2) Update only boot partition from generated boot.sd (CRITICAL: Do NOT use dd for this!)
sudo mkdir -p /mnt/sdc1
sudo mount /dev/sdc1 /mnt/sdc1
sudo cp /home/rgo/uni/kitfeup/duo-buildroot-sdk-v2/install/soc_sg2000_milkv_duos_musl_riscv64_sd/boot.sd /mnt/sdc1/boot.sd
sudo umount /mnt/sdc1
sync
```

Current recommended command (boot update only):
```sh
sudo mkdir -p /mnt/sdc1 && sudo mount /dev/sdc1 /mnt/sdc1 && sudo cp /home/rgo/uni/kitfeup/duo-buildroot-sdk-v2/install/soc_sg2000_milkv_duos_musl_riscv64_sd/boot.sd /mnt/sdc1/boot.sd && sudo umount /mnt/sdc1 && sync
```

Important: `of=/dev/sdc` writes the whole card; but when updating `boot.sd` alone, you must mount the VFAT partition and copy the file. Using `dd` to write a file directly to the partition destroys the VFAT structure!

### Build Only Parts
```sh
make build-libumdp
make build-umdp-ko
make build-shared
```

After changing timer IRQ/DT behavior, always rebuild `umdp.ko` and re-sync it to the board.

### Sync to Milk-V
```sh
make sync-compiled
make sync-umdp-ko
make sync-all
```

### Smoke Test on Board
```sh
make test-smoke
```

`make test-smoke` auto-loads `shared/umdp.ko` if module `umdp` is not loaded yet.

### Configuring for Your Machine
Override board connection values:

```sh
make sync-all BOARD_HOST=192.168.42.1 BOARD_USER=root BOARD_PASS=milkv
```

If your SDK/toolchain path differs:

```sh
make build-umdp-ko KERNEL_BUILD_DIR=/path/to/kernel/build CROSS_PREFIX=/path/to/riscv64-unknown-linux-musl-
```

## 7. Project Structure

```
shared/
├── compiled/          # Built RISC-V binaries (deployed to board)
├── src/
│   ├── timer/         # Timer IRQ library (timer_irq.h/c)
│   ├── led/           # GPIO LED library (led_gpio.h/c)
│   ├── rtc/           # RTC library (rtc.h/c)
│   ├── others/        # Additional experimental drivers (PWM work-in-progress moved here)
│   ├── blink_timer.c  # Test program using timer + led libraries
│   ├── uptime.c       # Test program using rtc library
│   ├── blink.c        # sysfs LED blink (step 2)
│   ├── blink_umdp.c   # UMDP GPIO blink (step 4)
│   └── timer_wait.c   # Timer interrupt wait demo
├── umdp.ko            # Kernel module
└── Makefile           # Builds all src/ programs into compiled/
```

Libraries are in `shared/src/{timer,led,rtc}/`.
Test/demo programs are in `shared/src/` and link against those libraries.
All binaries output to `shared/compiled/`.

## 8. Run Programs

Programs are built into `shared/compiled/`. Run them on the board after loading `umdp.ko`.

### Step 2: Basic LED Blink (sysfs)
```sh
./shared/compiled/blink
```

### Step 4: LED Blink using UMDP with sleep()
```sh
./shared/compiled/blink_umdp
```

### Step 5: Timer Interrupt Test
```sh
./shared/compiled/timer_wait          # Default 1000ms period
./shared/compiled/timer_wait 500      # 500ms period
```

### Step 6: LED Blink using Timer Interrupts
```sh
./shared/compiled/blink_timer         # Default 500ms blink
./shared/compiled/blink_timer 1000    # 1000ms blink
```

### Read RTC Counter
```sh
./shared/compiled/uptime
```

**Note:** On this image, the RTC block is not enabled by default at boot. This program enables the counter without setting a fake date, so the value shown is seconds since the RTC counter was first enabled (often first run after power cycle).

## 9. Hardware Reference

**SG2000 Limitations:**
- No hardware TRNG (removed from silicon)
- No internal temperature sensor
- Onboard LED (GPIOA[29]) has NO PWM alternate function
- RTC uses proprietary undocumented offsets
- SARADC is not accessible (clock gated, no IIO driver in kernel)

### GPIO
- **GPIO0 (porta)**: 0x03020000 (LED is pin 29, sysfs #509 = 480 + 29)
- **GPIO1 (portb)**: 0x03021000
- **GPIO2 (portc)**: 0x03022000
- **GPIO3 (portd)**: 0x03023000
- **GPIO4 (porte)**: 0x05021000

### GPIO Registers (per bank)
- 0x00: Data register
- 0x04: Direction register

### Timer (DW Timer 0)
- **Base**: 0x030A0000
- **Frequency**: 25MHz
- **Requested IRQ**: 79 (UMDP maps this to the Linux IRQ via PLIC domain)

### DT (Device Tree) notes
- **What is DT**: Device Tree is the hardware description the kernel uses at boot (addresses, interrupts, clocks, compatible strings).
- **Why it mattered here**: without a proper timer DT node at `0x030A0000`, Linux had no stable IRQ mapping for that timer and UMDP previously fell back to polling.
- **What we changed**:
  - Added `timer1: timer@30a0000` (`snps,dw-apb-timer`) in `duo-buildroot-sdk-v2/build/boards/default/dts/cv181x/cv181x_base.dtsi`.
  - Added interrupt wiring in `duo-buildroot-sdk-v2/build/boards/default/dts/cv181x_riscv/cv181x_base_riscv.dtsi` with PLIC IRQ 79.
  - Added `make build-board-image` to package updated board images after DT changes.

### Timer Registers
- 0x00: Load register
- 0x04: Current value
- 0x08: Control register (bit 0: enable, bit 1: periodic, bit 2: interrupt MASK 1=masked/0=unmasked)
- 0x0C: End of interrupt (read to clear)
- 0x10: Interrupt status

### Watchdog Timer (WDT)
- **Base**: 0x03010000

### PWM Controllers
- **PWM0**: 0x03060000
- **PWM1**: 0x03061000
- **PWM2**: 0x03062000
- **PWM3**: 0x03063000
- **Channels per controller**: 4
- **Timeout**: 2^(16+range) / 25MHz
- **Range 0x0B**: ~5.36 seconds
- **Feed password**: 0x76
- 0x00: Control register (bit 0: enable)
- 0x04: Timeout range
- 0x0C: Counter restart (feed the dog here)

### SARADC (Analog-to-Digital Converter)
- **Base**: 0x030F0000
- **Channels**: 0=ADC1, 1=ADC2, 2=ADC3 (external pins only!)
- 0x04: Control (bits 4-6: channel, bit 0: start)
- 0x08: Status (bit 0: done)
- 0x14: Result (12-bit value)

### NOT AVAILABLE ON SG2000:
- **RTC**: Proprietary offsets, use C time.h instead
- **TRNG**: Removed from silicon, use ADC noise instead
- **Internal Temp Sensor**: Does not exist, use external NTC on ADC

## 10. Useful Commands

### On Host
```sh
# Build everything
make build-all

# Sync to board
make sync-all

# Quick smoke test
make test-smoke

# SSH to board
./connect.sh
```

### On Board
```sh
# Load kernel module
insmod shared/umdp.ko

# Unload kernel module
rmmod umdp

# Check kernel messages
dmesg | tail -20

# List GPIO
ls /sys/class/gpio/
```

## Troubleshooting

### Module won't load
```sh
dmesg | tail -20
```

### Can't connect via SSH
- Check USB cable connection
- Verify IP: `ping 192.168.42.1`
- Try power cycling the board

### LED not blinking
- Ensure blink.sh is disabled: `ls /mnt/system/blink.sh_backup`
- Check GPIO export: `ls /sys/class/gpio/gpio509`
- Verify kernel module loaded: `lsmod | grep umdp`

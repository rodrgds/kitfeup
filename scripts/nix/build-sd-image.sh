#!/usr/bin/env bash
set -eo pipefail

source duo-buildroot-sdk-v2/build/envsetup_milkv.sh
lunch milkv-duos-sd
build_kernel
pack_sd_image

echo "OUTPUT_DIR=$OUTPUT_DIR"
ls -la "$OUTPUT_DIR"
ls -la "$OUTPUT_DIR"/rawimages 2>/dev/null || true

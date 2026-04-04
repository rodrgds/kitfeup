#!/usr/bin/env bash
set -euo pipefail

DEVICE="${DEVICE:-}"
IMAGE="${IMAGE:-}"

if [ -z "$DEVICE" ]; then
  echo "[image-flash] DEVICE is required (e.g., /dev/sdX)" >&2
  exit 1
fi

if [ ! -b "$DEVICE" ]; then
  echo "[image-flash] Not a block device: $DEVICE" >&2
  exit 1
fi

if [ -z "$IMAGE" ]; then
  CANDIDATE_IMG=$(ls -1 "$ROOT_DIR"/downloads/*.img 2>/dev/null | head -n 1 || true)
  CANDIDATE_XZ=$(ls -1 "$ROOT_DIR"/downloads/*.img.xz 2>/dev/null | head -n 1 || true)
  CANDIDATE_ZIP=$(ls -1 "$ROOT_DIR"/downloads/*.img.zip 2>/dev/null | head -n 1 || true)
  if [ -n "$CANDIDATE_IMG" ]; then
    IMAGE="$CANDIDATE_IMG"
  elif [ -n "$CANDIDATE_XZ" ]; then
    IMAGE="$CANDIDATE_XZ"
  elif [ -n "$CANDIDATE_ZIP" ]; then
    IMAGE="$CANDIDATE_ZIP"
  else
    echo "[image-flash] IMAGE not provided and no image found in $ROOT_DIR/downloads" >&2
    exit 1
  fi
fi

if [ ! -f "$IMAGE" ]; then
  echo "[image-flash] IMAGE not found: $IMAGE" >&2
  exit 1
fi

IMAGE_BASENAME=$(basename "$IMAGE")
if [ "$IMAGE_BASENAME" = "boot.sd" ] || [[ "$IMAGE_BASENAME" == *.sd ]]; then
  echo "[image-flash] Refusing to flash boot artifact '$IMAGE_BASENAME' with dd." >&2
  echo "[image-flash] boot.sd must be copied to the boot partition, not raw-flashed." >&2
  echo "[image-flash] Use: ./kitfeup image install-boot /dev/sdX1" >&2
  exit 1
fi

DEVICE_SIZE_BYTES=""
DEVICE_SIZE_GB="unknown"
if [ -r "/sys/class/block/$(basename "$DEVICE")/size" ]; then
  SECTORS=$(cat "/sys/class/block/$(basename "$DEVICE")/size" 2>/dev/null || true)
  if [[ "$SECTORS" =~ ^[0-9]+$ ]]; then
    DEVICE_SIZE_BYTES=$((SECTORS * 512))
    DEVICE_SIZE_GB=$(awk "BEGIN { printf \"%.1f\", ${DEVICE_SIZE_BYTES}/1024/1024/1024 }")
  fi
fi

echo "[image-flash] About to flash"
echo "  IMAGE:  $IMAGE"
echo "  DEVICE: $DEVICE"
echo "  SIZE:   ${DEVICE_SIZE_GB} GB"
echo "[image-flash] This will overwrite the target device."

if [ "${KITFEUP_FORCE_FLASH:-0}" != "1" ]; then
  printf "Proceed with flash? [y/N]: "
  read -r CONFIRM
  case "$CONFIRM" in
    y|Y|yes|YES)
      ;;
    *)
      echo "[image-flash] Aborted by user (default: no)." >&2
      exit 1
      ;;
  esac
fi

if [[ "$IMAGE" == *.zip ]]; then
  echo "[image-flash] Writing zipped image with unzip -p..."
  unzip -p "$IMAGE" | sudo dd of="$DEVICE" bs=4M conv=fsync status=progress
elif [[ "$IMAGE" == *.xz ]]; then
  echo "[image-flash] Writing compressed image with xzcat..."
  xzcat "$IMAGE" | sudo dd of="$DEVICE" bs=4M conv=fsync status=progress
else
  echo "[image-flash] Writing raw image..."
  sudo dd if="$IMAGE" of="$DEVICE" bs=4M conv=fsync status=progress
fi

sync
echo "[image-flash] done"

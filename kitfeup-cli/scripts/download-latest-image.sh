#!/usr/bin/env bash
set -euo pipefail

IMAGE_REPO="${IMAGE_REPO:-milkv-duo/duo-buildroot-sdk-v2}"
DOWNLOAD_DIR="${DOWNLOAD_DIR:-$ROOT_DIR/downloads}"
IMAGE_PATTERN="${IMAGE_PATTERN:-*duos*musl*riscv64*sd*.img.zip}"

if ! command -v gh >/dev/null 2>&1; then
  echo "[image-download] gh CLI not found. Install GitHub CLI first." >&2
  exit 1
fi

mkdir -p "$DOWNLOAD_DIR"

echo "[image-download] repo: $IMAGE_REPO"
echo "[image-download] pattern: $IMAGE_PATTERN"
echo "[image-download] downloading latest release assets to: $DOWNLOAD_DIR"

gh release download --repo "$IMAGE_REPO" --dir "$DOWNLOAD_DIR" --clobber --pattern "$IMAGE_PATTERN"

echo "[image-download] done"
ls -lh "$DOWNLOAD_DIR" | sed -n '1,80p'

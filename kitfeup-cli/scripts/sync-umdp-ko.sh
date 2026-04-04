#!/usr/bin/env bash
set -euo pipefail

echo "[sync-umdp-ko] uploading $ROOT_DIR/shared/umdp.ko"

sshpass -p "$BOARD_PASS" scp -o StrictHostKeyChecking=no \
  "$ROOT_DIR/shared/umdp.ko" \
  "$BOARD_USER@$BOARD_HOST:$BOARD_SHARED_DIR/umdp.ko"

echo "[sync-umdp-ko] done"

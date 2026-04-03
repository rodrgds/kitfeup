#!/usr/bin/env bash
set -euo pipefail

echo "[sync-compiled] creating remote directory: $BOARD_SHARED_DIR/compiled"

sshpass -p "$BOARD_PASS" ssh -n -o StrictHostKeyChecking=no "$BOARD_USER@$BOARD_HOST" \
  "mkdir -p $BOARD_SHARED_DIR/compiled && rm -f $BOARD_SHARED_DIR/compiled/*"

echo "[sync-compiled] uploading binaries from $ROOT_DIR/shared/compiled"

sshpass -p "$BOARD_PASS" scp -o StrictHostKeyChecking=no \
  "$ROOT_DIR/shared/compiled"/* \
  "$BOARD_USER@$BOARD_HOST:$BOARD_SHARED_DIR/compiled/"

echo "[sync-compiled] done"

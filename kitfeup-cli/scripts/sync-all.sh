#!/usr/bin/env bash
set -euo pipefail

SRC_DIR="$ROOT_DIR/shared"
BIN_DIR="$SRC_DIR/compiled"
KO_FILE="$SRC_DIR/umdp.ko"
REMOTE="$BOARD_USER@$BOARD_HOST"
REMOTE_DIR="$BOARD_SHARED_DIR"

if [ ! -d "$BIN_DIR" ]; then
  echo "[sync-all] Missing compiled directory: $BIN_DIR" >&2
  exit 1
fi

if [ ! -f "$KO_FILE" ]; then
  echo "[sync-all] Missing module file: $KO_FILE" >&2
  exit 1
fi

echo "[sync-all] ensuring remote directories"
sshpass -p "$BOARD_PASS" ssh -n -o StrictHostKeyChecking=no "$REMOTE" \
  "mkdir -p '$REMOTE_DIR/compiled' && rm -f '$REMOTE_DIR/compiled/'*"

echo "[sync-all] syncing compiled binaries (scp)"
sshpass -p "$BOARD_PASS" scp -o StrictHostKeyChecking=no \
  "$BIN_DIR"/* "$REMOTE:$REMOTE_DIR/compiled/"

echo "[sync-all] syncing umdp.ko"
sshpass -p "$BOARD_PASS" scp -o StrictHostKeyChecking=no \
  "$KO_FILE" "$REMOTE:$REMOTE_DIR/umdp.ko"

echo "[sync-all] done"

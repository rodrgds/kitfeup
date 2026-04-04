#!/usr/bin/env bash
set -euo pipefail

BIN="$ROOT_DIR/kitfeup-cli/bin/kitfeupb"
REMOTE="$BOARD_USER@$BOARD_HOST"
DEST_DIR="$BOARD_SHARED_DIR"

if [ ! -f "$BIN" ]; then
  echo "[sync-kitfeupb] Missing binary: $BIN" >&2
  exit 1
fi

echo "[sync-kitfeupb] ensuring remote directory: $DEST_DIR"
sshpass -p "$BOARD_PASS" ssh -n -o StrictHostKeyChecking=no "$REMOTE" \
  "mkdir -p '$DEST_DIR'"

echo "[sync-kitfeupb] uploading $BIN -> $DEST_DIR/kitfeupb"
sshpass -p "$BOARD_PASS" scp -o StrictHostKeyChecking=no \
  "$BIN" "$REMOTE:$DEST_DIR/kitfeupb"

echo "[sync-kitfeupb] ensuring board PATH has $DEST_DIR"
sshpass -p "$BOARD_PASS" ssh -n -o StrictHostKeyChecking=no "$REMOTE" \
  "for f in /root/.profile /root/.bashrc; do \
     [ -f \"\$f\" ] || touch \"\$f\"; \
     grep -q 'export PATH=\"$DEST_DIR:\$PATH\"' \"\$f\" || echo 'export PATH=\"$DEST_DIR:\$PATH\"' >> \"\$f\"; \
   done"

echo "[sync-kitfeupb] done"

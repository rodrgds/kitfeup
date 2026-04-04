#!/usr/bin/env bash
set -euo pipefail

FULL_CLEAN="${FULL_CLEAN:-0}"

if [ "$FULL_CLEAN" = "1" ]; then
  make -C "$ROOT_DIR/shared" clean
fi

make -C "$ROOT_DIR/shared" all

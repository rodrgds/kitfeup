#!/usr/bin/env bash
set -euo pipefail

make -C "$ROOT_DIR/shared" clean
make -C "$ROOT_DIR/shared" all

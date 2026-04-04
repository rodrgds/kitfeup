#!/usr/bin/env bash
set -e

FILE="$1"
if [ ! -f "$FILE" ]; then
    echo "Usage: $0 <boot.sd file>"
    exit 1
fi

MAGIC=$(head -c 4 "$FILE")
if [ "$MAGIC" = "CIMG" ]; then
    echo "Found CIMG header. Stripping 128 bytes to extract FIT image..."
    dd if="$FILE" of="${FILE}.fit" bs=128 skip=1 status=none
    mv "${FILE}.fit" "$FILE"
    echo "CIMG header removed. File is now a standard FIT image."
else
    echo "No CIMG header found. File left as is."
fi

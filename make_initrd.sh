#!/bin/bash
# Build initrd image from initrd/ directory
set -e

OUT="build/initrd"
TMPDIR=$(mktemp -d)

# Collect files
HEADER_SIZE=8
ENTRIES_SIZE=0
FILES=""
OFFSET=$((HEADER_SIZE))

# Count files and compute entries size
file_count=0
for f in initrd/*; do
    [ -f "$f" ] || continue
    file_count=$((file_count + 1))
done

ENTRIES_SIZE=$((file_count * 36))  # name(32) + offset(4) + size(4) = 40, but struct is 36 (packed? no, 32+4+4=40)

# Let's use 40 byte entries
ENTRIES_SIZE=$((file_count * 40))

# Write header
printf '\x46\x53\x4F\x4E' > "$TMPDIR/initrd.tmp"  # "NOSF" little endian magic 0x4E4F5346
printf "\\x$(printf '%02x' $((file_count & 0xFF)))\\x$(printf '%02x' $(( (file_count>>8)&0xFF)))\\x$(printf '%02x' $(( (file_count>>16)&0xFF)))\\x$(printf '%02x' $(( (file_count>>24)&0xFF)))" >> "$TMPDIR/initrd.tmp"

# Write entries then file data
DATA_OFFSET=$((HEADER_SIZE + ENTRIES_SIZE))
CUR_OFFSET=$DATA_OFFSET

for f in initrd/*; do
    [ -f "$f" ] || continue
    fname=$(basename "$f")
    fsize=$(stat -c%s "$f")
    # Pad name to 32 bytes
    name_padded=$(printf "%-32s" "$fname" | head -c 32)
    printf "%s" "$name_padded" >> "$TMPDIR/initrd.tmp"
    printf "\\x$(printf '%02x' $((CUR_OFFSET & 0xFF)))\\x$(printf '%02x' $(( (CUR_OFFSET>>8)&0xFF)))\\x$(printf '%02x' $(( (CUR_OFFSET>>16)&0xFF)))\\x$(printf '%02x' $(( (CUR_OFFSET>>24)&0xFF)))" >> "$TMPDIR/initrd.tmp"
    printf "\\x$(printf '%02x' $((fsize & 0xFF)))\\x$(printf '%02x' $(( (fsize>>8)&0xFF)))\\x$(printf '%02x' $(( (fsize>>16)&0xFF)))\\x$(printf '%02x' $(( (fsize>>24)&0xFF)))" >> "$TMPDIR/initrd.tmp"
    CUR_OFFSET=$((CUR_OFFSET + fsize))
done

# Pad to data offset
while [ $(stat -c%s "$TMPDIR/initrd.tmp") -lt $DATA_OFFSET ]; do
    printf '\x00' >> "$TMPDIR/initrd.tmp"
done

# Append file data
for f in initrd/*; do
    [ -f "$f" ] || continue
    cat "$f" >> "$TMPDIR/initrd.tmp"
done

cp "$TMPDIR/initrd.tmp" "$OUT"
rm -rf "$TMPDIR"
echo "initrd built: $OUT ($(stat -c%s "$OUT") bytes, $file_count files)"

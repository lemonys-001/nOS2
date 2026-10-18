#!/bin/bash
# Build bootable hard disk image for Limbo Emulator
set -e

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$PROJECT_DIR"

KERNEL="build/kernel.elf"
IMG="nOS.img"
FATIMG="build/fatpart.img"
COREIMG="build/core.img"

# Build kernel first
make "$KERNEL" > /dev/null 2>&1

# Create 30MB FAT16 partition image
rm -f "$FATIMG"
dd if=/dev/zero of="$FATIMG" bs=1M count=30 2>/dev/null
mkfs.vfat -F 16 -n "NOSHDD" "$FATIMG" > /dev/null 2>&1

# Copy kernel and grub.cfg into FAT partition using mtools
mmd -i "$FATIMG" ::/boot 2>/dev/null
mmd -i "$FATIMG" ::/boot/grub 2>/dev/null
mcopy -i "$FATIMG" "$KERNEL" ::/boot/ 2>/dev/null
mcopy -i "$FATIMG" iso/boot/grub/grub.cfg ::/boot/grub/ 2>/dev/null

# Create main disk image
rm -f "$IMG"
dd if=/dev/zero of="$IMG" bs=1M count=32 2>/dev/null

# Write GRUB boot sector to MBR
dd if=/usr/lib/grub/i386-pc/boot.img of="$IMG" bs=512 count=1 conv=notrunc 2>/dev/null

# Create and embed GRUB core image
grub-mkimage -d /usr/lib/grub/i386-pc -O i386-pc -p '(hd0,msdos1)/boot/grub' -o "$COREIMG" fat multiboot 2>&1
dd if="$COREIMG" of="$IMG" bs=512 seek=1 conv=notrunc 2>/dev/null

# Write FAT partition at sector 2048 (1MB offset)
dd if="$FATIMG" of="$IMG" bs=512 seek=2048 conv=notrunc 2>/dev/null

# Write MBR partition table
python3 -c "
import struct
fat16_type = 0x06
lba_start = 2048
lba_size = 30 * 1024 * 1024 // 512
part_entry = struct.pack('<BBBBBBBBII',
    0x80, 0, 1, 0, fat16_type, 0xff, 0xff, 0xff, lba_start, lba_size
)
partition_table = part_entry + b'\x00' * 48
mbr_sig = b'\x55\xAA'
with open('$IMG', 'r+b') as f:
    f.seek(446)
    f.write(partition_table)
    f.seek(510)
    f.write(mbr_sig)
"

# Cleanup intermediate files
rm -f "$FATIMG" "$COREIMG"

echo "Hard disk image created: $IMG"
echo "Use in Limbo: Machine=PC(i386), RAM=256MB, Storage=nOS.img, Boot=Hard Disk"

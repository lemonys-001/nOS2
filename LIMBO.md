# Running nOS in Limbo Emulator (Android)

## The Problem
Limbo Emulator's BIOS couldn't boot the previous ISO because it used a raw kernel as the El Torito boot image (`-no-emul-boot`), which isn't a valid PC boot sector. Limbo got stuck on "booting from CD/DVD" because the BIOS didn't know how to load it.

## The Fix
The ISO now uses **GRUB2** (`grub-mkrescue`), which writes a proper PC boot sector (`boot_hybrid.img`) that Limbo's BIOS understands.

## Steps to Run in Limbo

1. **Copy the ISO to your Android device**
   - Transfer `nOS.iso` (from `nOS-02/` or `nOS2/`) to your phone's storage (e.g., `/sdcard/Download/nOS.iso`)

2. **Create a new VM in Limbo**
   - Open Limbo Emulator
   - Tap "+" to create a new machine
   - **Machine:** PC (i386)
   - **RAM:** 256 MB (minimum)
   - **Storage:** Create a new virtual disk (any size, e.g., 2 GB)
   - **CD/DVD:** Insert `nOS.iso`
   - **Boot order:** CD/DVD first

3. **Start the VM**
   - Tap the play button
   - You should see the GRUB menu, then nOS boots to the Windows 7-style desktop

4. **If it still gets stuck on "booting from CD/DVD"**
   - Make sure you're using the **new** `nOS.iso` (built with GRUB), not the old one
   - In Limbo VM settings, set **Boot Order** to CD/DVD only (disable hard disk boot temporarily)
   - Try **Machine: i386** (not x86_64)
   - Ensure **CD/DVD** is pointed at `nOS.iso`

## Alternative: Boot from Floppy (most reliable in Limbo)

If CD/DVD still doesn't work, use a floppy image:

```bash
# Create a 1.44MB FAT12 floppy with GRUB
dd if=/dev/zero of=nOS.flp bs=1024 count=1440
mkfs.msdos nOS.flp
mkdir -p /tmp/flop
sudo mount -o loop nOS.flp /tmp/flop
mkdir -p /tmp/flop/boot/grub
cp build/kernel.bin /tmp/flop/boot/
cp iso/boot/grub/grub.cfg /tmp/flop/boot/grub/
grub-mkimage -d /usr/lib/grub/i386-pc -o /tmp/flop/boot/grub/core.img -O i386-pc -p '(fd0)/boot/grub' biosdisk fat
sudo umount /tmp/flop
```

Then in Limbo, attach `nOS.flp` as the floppy and set boot order to floppy first.

## Limbo Settings That Help
- **Display:** SDL (default) or VNC
- **Acceleration:** TCG (no KVM needed on Android)
- **Machine type:** pc (i386)
- **BIOS:** Default (SeaBIOS)

## Files
- `nOS.iso` — Bootable CD image (use this in Limbo's CD/DVD slot)
- `build/kernel.bin` — Raw kernel (for advanced/custom boot setups)

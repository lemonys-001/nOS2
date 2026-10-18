# Running nOS in Limbo Emulator (Android)

## The Problem
Limbo Emulator's virtual CD-ROM couldn't boot the GRUB ISO ("couldn't read the boot disk"), and earlier ISOs had ELF/multiboot module issues.

## The Solution: Hard Disk Image
Use `nOS.img` — a 32MB bootable hard disk image with GRUB installed to the MBR. Limbo boots hard disks much more reliably than CD-ROMs.

## Files in nOS-02/
- **`nOS.img`** — Bootable hard disk image (USE THIS in Limbo)
- **`nOS.iso`** — Bootable CD image (alternative, if CD-ROM works in your Limbo version)

## Steps to Run in Limbo

1. **Copy files to your Android device**
   - Transfer `nOS.img` (and `nOS.iso` as backup) to your phone's storage (e.g., `/sdcard/Download/`)

2. **Create a new VM in Limbo**
   - Open Limbo Emulator
   - Tap "+" to create a new machine
   - **Machine:** PC (i386)
   - **RAM:** 256 MB
   - **Storage:** Tap the storage slot → Insert image → Select `nOS.img`
   - **Boot order:** Hard disk first

3. **Start the VM**
   - Tap the play button
   - GRUB loads `kernel.elf` via multiboot, then nOS boots to the Windows 7-style desktop

4. **If it still doesn't boot**
   - Make sure you're using `nOS.img` as the **Hard Disk** (not CD/DVD)
   - Set **Boot Order** to Hard Disk only
   - Try **Machine: i386** (not x86_64)
   - **Acceleration:** TCG (default on Android, no KVM needed)

## Limbo Settings Summary
| Setting | Value |
|---------|-------|
| Machine | PC (i386) |
| RAM | 256 MB |
| Storage | nOS.img (Hard Disk) |
| Boot Order | Hard Disk first |
| Acceleration | TCG |

## Alternative: CD-ROM (if your Limbo supports it)
Use `nOS.iso` in the CD/DVD slot with Boot Order set to CD/DVD first.

## Files
- `nOS.img` — Bootable hard disk image (recommended for Limbo)
- `nOS.iso` — Bootable CD image (alternative)
- `build/kernel.elf` — Raw kernel ELF (for advanced/custom boot setups)

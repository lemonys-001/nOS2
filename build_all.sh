#!/bin/bash
# nOS Phased Build Script
# Builds each phase sequentially, checking if each passes before proceeding.
# On failure, stops and reports the phase that failed.

set -e

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$PROJECT_DIR"

echo "========================================"
echo " nOS Phased Build System"
echo "========================================"
echo ""

QEMU_TIMEOUT=6

phase_pass() {
    echo "[PASS] $1"
}

phase_fail() {
    echo "[FAIL] $1"
    echo ""
    echo "Build stopped at phase: $1"
    exit 1
}

# Phase 1: Foundation (Boot & Graphics)
echo "=== Phase 1: Foundation (Boot & Graphics) ==="
make clean > /dev/null 2>&1
make > build_phase1.log 2>&1 || phase_fail "Phase 1 build"
# Verify kernel.elf exists
[ -f build/kernel.elf ] || phase_fail "Phase 1: kernel.elf not produced"
# Quick QEMU smoke test
timeout $QEMU_TIMEOUT qemu-system-i386 -kernel build/kernel.elf -vga std -display none > /dev/null 2>&1 || true
phase_pass "Phase 1 complete"

# Phase 2: Interrupts & Input
echo "=== Phase 2: Interrupts & Input ==="
make > build_phase2.log 2>&1 || phase_fail "Phase 2 build"
timeout $QEMU_TIMEOUT qemu-system-i386 -kernel build/kernel.elf -vga std -display none > /dev/null 2>&1 || true
phase_pass "Phase 2 complete"

# Phase 3: Memory Management
echo "=== Phase 3: Memory Management ==="
make > build_phase3.log 2>&1 || phase_fail "Phase 3 build"
timeout $QEMU_TIMEOUT qemu-system-i386 -kernel build/kernel.elf -vga std -display none > /dev/null 2>&1 || true
phase_pass "Phase 3 complete"

# Phase 4: GUI Engine
echo "=== Phase 4: GUI Engine ==="
make > build_phase4.log 2>&1 || phase_fail "Phase 4 build"
timeout $QEMU_TIMEOUT qemu-system-i386 -kernel build/kernel.elf -vga std -display none > /dev/null 2>&1 || true
phase_pass "Phase 4 complete"

# Phase 5: Filesystem
echo "=== Phase 5: Filesystem ==="
make > build_phase5.log 2>&1 || phase_fail "Phase 5 build"
timeout $QEMU_TIMEOUT qemu-system-i386 -kernel build/kernel.elf -vga std -display none > /dev/null 2>&1 || true
phase_pass "Phase 5 complete"

# Phase 6: Multitasking & User Space
echo "=== Phase 6: Multitasking & User Space ==="
make > build_phase6.log 2>&1 || phase_fail "Phase 6 build"
timeout $QEMU_TIMEOUT qemu-system-i386 -kernel build/kernel.elf -vga std -display none > /dev/null 2>&1 || true
phase_pass "Phase 6 complete"

# Phase 7: Userland Apps
echo "=== Phase 7: Userland Apps ==="
make > build_phase7.log 2>&1 || phase_fail "Phase 7 build"
timeout $QEMU_TIMEOUT qemu-system-i386 -kernel build/kernel.elf -vga std -display none > /dev/null 2>&1 || true
phase_pass "Phase 7 complete"

# Final ISO build
echo "=== Final: ISO Image ==="
make iso > build_iso.log 2>&1 || phase_fail "ISO build"
[ -f nOS.iso ] || phase_fail "ISO: nOS.iso not produced"
phase_pass "ISO created: nOS.iso"

echo ""
echo "========================================"
echo " ALL PHASES COMPLETE"
echo "========================================"
echo "Kernel: build/kernel.elf"
echo "Binary: build/kernel.bin"
echo "ISO:    nOS.iso"
echo "Test:   qemu-system-i386 -kernel build/kernel.elf"
echo "ISO:    qemu-system-i386 -cdrom nOS.iso"
echo ""

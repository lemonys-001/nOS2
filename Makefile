CC      := i686-linux-gnu-gcc-12
AS      := nasm
LD      := i686-linux-gnu-ld
OBJCOPY := i686-linux-gnu-objcopy
QEMU    := qemu-system-i386

CFLAGS  := -ffreestanding -fno-stack-protector -nostdlib -nostdinc -fno-builtin -fno-exceptions -Wall -Wextra -Werror -I. -I/usr/i686-linux-gnu/include
ASFLAGS := -f elf32
LDFLAGS := -T src/linker.ld -nostdlib -m elf_i386

SRC_DIR := src
BUILD_DIR := build

KERNEL_ELF := $(BUILD_DIR)/kernel.elf
KERNEL_BIN := $(BUILD_DIR)/kernel.bin

SRCS := $(wildcard $(SRC_DIR)/*.c)
ASMS := $(wildcard $(SRC_DIR)/*.s)
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o) $(ASMS:$(SRC_DIR)/%.s=$(BUILD_DIR)/%.o)

INITRD_BIN := $(BUILD_DIR)/initrd
INITRD_OBJ := $(BUILD_DIR)/initrd.o

.PHONY: all clean run iso phase-check initrd

all: initrd $(KERNEL_ELF) $(KERNEL_BIN)

initrd: $(INITRD_BIN)

$(INITRD_BIN): | $(BUILD_DIR)
	bash make_initrd.sh

$(INITRD_OBJ): $(INITRD_BIN) | $(BUILD_DIR)
	$(OBJCOPY) -I binary -O elf32-i386 $< $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

$(KERNEL_ELF): $(OBJS) $(INITRD_OBJ) src/linker.ld | $(BUILD_DIR)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(INITRD_OBJ)

$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

run: $(KERNEL_ELF)
	$(QEMU) -kernel $(KERNEL_ELF) -vga std -serial stdio -m 256

iso: $(KERNEL_ELF)
	mkdir -p iso/boot/grub
	cp $(KERNEL_ELF) iso/boot/kernel.elf
	grub-mkrescue --modules="multiboot fat" -o nOS.iso iso/
	@echo "ISO created: nOS.iso"

hdd:
	bash make_hdd.sh
	@echo "Hard disk image created: nOS.img"

phase-check: all
	@echo "Phase build OK: $(KERNEL_ELF)"

clean:
	rm -rf $(BUILD_DIR) nOS.iso
	rm -f iso/boot/kernel.bin
	rmdir iso/boot/grub iso/boot iso 2>/dev/null || true

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

BOOT_OBJ  := $(BUILD_DIR)/boot.o
KERNEL_OBJ := $(BUILD_DIR)/kernel.o

.PHONY: all clean run

all: $(KERNEL_ELF) $(KERNEL_BIN)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BOOT_OBJ): $(SRC_DIR)/boot.s | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

$(KERNEL_OBJ): $(SRC_DIR)/kernel.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL_ELF): $(BOOT_OBJ) $(KERNEL_OBJ) src/linker.ld | $(BUILD_DIR)
	$(LD) $(LDFLAGS) -o $@ $(BOOT_OBJ) $(KERNEL_OBJ)

$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

run: $(KERNEL_ELF)
	$(QEMU) -kernel $(KERNEL_ELF) -vga std -serial stdio

clean:
	rm -rf $(BUILD_DIR)

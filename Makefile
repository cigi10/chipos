# RISC-V OS Build Configuration
ARCH = riscv64-linux-gnu
CC = $(ARCH)-gcc
AS = $(ARCH)-as
LD = $(ARCH)-ld
OBJCOPY = $(ARCH)-objcopy

# Compiler flags
CFLAGS = -std=gnu11 -ffreestanding -O2 -Wall -Wextra -mcmodel=medany \
 -Ikernel/include -Ikernel/drivers -Ikernel/shell -Ikernel/editor \
 -Ilib -Ikernel/fs -I. \
 -D__riscv -D__riscv_xlen=64 \
 -nostdinc -fno-builtin \
 -isystem $(shell $(CC) -print-file-name=include)

ASFLAGS =
LDFLAGS = -nostdlib

# Directories
BUILD_DIR = build
KERNEL_DIR = kernel
DRIVERS_DIR = $(KERNEL_DIR)/drivers
MEMORY_DIR = $(KERNEL_DIR)/memory
SHELL_DIR = $(KERNEL_DIR)/shell
EDITOR_DIR = $(KERNEL_DIR)/editor
FS_DIR = $(KERNEL_DIR)/fs
LIB_DIR = lib

# Source files
ASM_SOURCES = boot/boot.s
C_SOURCES = $(KERNEL_DIR)/kernel.c \
$(DRIVERS_DIR)/console.c \
$(MEMORY_DIR)/memory.c \
$(SHELL_DIR)/shell.c \
$(EDITOR_DIR)/editor.c \
$(FS_DIR)/fs.c \
$(LIB_DIR)/string.c

# Object files - all in flat build directory
OBJECTS = $(addprefix $(BUILD_DIR)/, \
 $(notdir $(ASM_SOURCES:.s=.o)) \
 $(notdir $(C_SOURCES:.c=.o)))

# Final outputs
KERNEL_ELF = $(BUILD_DIR)/kernel.elf
KERNEL_BIN = $(BUILD_DIR)/kernel.bin

# Targets
all: $(KERNEL_BIN)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Assembly compilation
$(BUILD_DIR)/%.o: boot/%.s | $(BUILD_DIR)
	$(AS) $(ASFLAGS) -o $@ $<

# C compilation - all files go directly to build/
$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(DRIVERS_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(MEMORY_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SHELL_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(EDITOR_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(FS_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(LIB_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Linking
$(KERNEL_ELF): $(OBJECTS) boot/linker.ld
	$(LD) $(LDFLAGS) -T boot/linker.ld -o $@ $(OBJECTS)

$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

# Utilities
run: $(KERNEL_BIN)
	qemu-system-riscv64 -machine virt -bios none -kernel $(KERNEL_ELF) -nographic -serial mon:stdio

debug: $(KERNEL_ELF)
	qemu-system-riscv64 -machine virt -bios none -kernel $(KERNEL_ELF) -serial stdio -nographic -s -S &
	$(ARCH)-gdb $(KERNEL_ELF) -ex "target remote :1234"

clean:
	rm -rf $(BUILD_DIR)

objdump: $(KERNEL_ELF)
	$(ARCH)-objdump -h $<

disasm: $(KERNEL_ELF)
	$(ARCH)-objdump -d $<

.PHONY: all run debug clean objdump disasm
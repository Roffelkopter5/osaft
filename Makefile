ASM=nasm
VM=qemu-system-i386
SRC_DIR=src
BUILD_DIR=build

.PHONY: run floppy_image kernel bootloader clean always

# floppy image
floppy_image: $(BUILD_DIR)/main_floppy.img

$(BUILD_DIR)/main_floppy.img: bootloader kernel
	dd if=/dev/zero of=$(BUILD_DIR)/main_floppy.img bs=512 count=2880
	mkfs.fat -F 12 -n "NBOS" $(BUILD_DIR)/main_floppy.img
	dd if=$(BUILD_DIR)/bootloader.bin of=$(BUILD_DIR)/main_floppy.img conv=notrunc
	mcopy -i $(BUILD_DIR)/main_floppy.img $(BUILD_DIR)/kernel.bin "::kernel.bin"

# bootloader
bootloader: $(BUILD_DIR)/bootloader.bin

$(BUILD_DIR)/bootloader.bin:$(SRC_DIR)/bootloader/boot.asm always
	$(ASM) $< -f bin -o $@

# kernel
kernel: $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin: $(SRC_DIR)/kernel/main.asm always
	$(ASM) $< -f bin -o $@

run:
	$(VM) -fda $(BUILD_DIR)/main_floppy.img

always:
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf build/
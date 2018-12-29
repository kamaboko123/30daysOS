AS=gcc
LINK_SCRIPT=link.ls
AS_OPT=-nostdlib -T $(LINK_SCRIPT)
SRC=ipl.s
TARGET_DIR=bin
TARGET_IMG=$(TARGET_DIR)/helloos.img
TARGET_OBJ=$(TARGET_DIR)/ipl.bin
TARGET_IMG_SIZE=1474560

QEMU=qemu-system-i386
#QEMU=qemu-system-x86_64

all:img

img:asm
	dd if=$(TARGET_OBJ) of=$(TARGET_IMG) bs=1
	#padding
	dd if=/dev/zero of=$(TARGET_IMG) bs=1 count=0 seek=$(TARGET_IMG_SIZE)

asm:
	mkdir -p $(TARGET_DIR)
	$(AS) $(AS_OPT) $(SRC) -o $(TARGET_OBJ)

$(TARGET_IMG):all

run: $(TARGET_IMG)
	$(QEMU) -drive format=raw,file=$(TARGET_IMG),if=floppy

clean:
	rm -rf $(TARGET_DIR)


AS=as
SRC=helloos2.s
TARGET_DIR=bin
TARGET_OBJ=$(TARGET_DIR)/helloos2.o
TARGET_IMG=$(TARGET_DIR)/helloos2.img
IMG_SIZE=512

QEMU=qemu-system-i386
QEMU_OPT=

all:
	mkdir -p $(TARGET_DIR)
	$(AS) $(SRC) -o $(TARGET_OBJ)
	$(eval ELFHEAD_SIZE := $(shell readelf -h bin/helloos2.o | grep "Size of this" | grep -o -E "[0-9]+"))
	dd if=$(TARGET_OBJ) of=$(TARGET_IMG) skip=$(ELFHEAD_SIZE) bs=1 count=$(IMG_SIZE)

$(TARGET_IMG):all

run: $(TARGET_IMG)
	$(QEMU) -fda $(TARGET_IMG)

clean:
	rm -rf $(TARGET_DIR)


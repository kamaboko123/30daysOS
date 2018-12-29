AS=gcc
AS_OPT=-nostdlib -T link.ls
SRC=helloos3.s
TARGET_DIR=bin
TARGET_IMG=$(TARGET_DIR)/helloos3.img

QEMU=qemu-system-i386

all:
	mkdir -p $(TARGET_DIR)
	$(AS) $(AS_OPT) $(SRC) -o $(TARGET_IMG)

$(TARGET_IMG):all

run: $(TARGET_IMG)
	$(QEMU) -drive format=raw,file=$(TARGET_IMG),if=floppy

clean:
	rm -rf $(TARGETE_DIR)

#gcc -nostdlib -o bin/helloos3.img -Tlink.ls helloos3.s
#qemu-system-i386 -drive format=raw,file=bin/helloos3.img,if=floppy

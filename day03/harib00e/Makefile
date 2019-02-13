AS=gcc
LINK_SCIRPT=link.lds

IPL_SRC=ipl.s
OS_SRC=haribote.s

TARGET_DIR=bin
IPL_BIN=$(TARGET_DIR)/ipl.bin
OS_BIN=$(TARGET_DIR)/haribote.sys
TARGET_IMG=$(TARGET_DIR)/haribote.img

QEMU=qemu-system-x86_64

all: $(TARGET_IMG)

$(OS_BIN): $(OS_SRC) $(LINK_SCIRPT)
	mkdir -p $(TARGET_DIR)
	gcc -nostdlib -o $(OS_BIN) -T $(LINK_SCIRPT) $(OS_SRC)

$(IPL_BIN): $(IPL_SRC) $(LINK_SCIRPT)
	mkdir -p $(TARGET_DIR)
	gcc -nostdlib -o $(IPL_BIN) -T $(LINK_SCIRPT) $(IPL_SRC)

$(TARGET_IMG): $(OS_BIN) $(IPL_BIN)
	#イメージ作成、IPLをブートセクタに配置
	mformat -f 1440 -B $(IPL_BIN) -C -i $(TARGET_IMG) ::
	#OSのプログラムをイメージにコピーする
	mcopy $(OS_BIN) -i $(TARGET_IMG) ::

run: all
	#$(QEMU) -drive format=raw,file=$(TARGET_IMG),if=floppy
	hexdump $(TARGET_IMG)

clean:
	rm -rf $(TARGET_DIR)

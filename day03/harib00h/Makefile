AS=gcc
IPL_LINK_SCRIPT=ipl.lds
OS_LINK_SCRIPT=os.lds

IPL_SRC=ipl.s
OS_SRC=haribote.s

TARGET_DIR=bin
IPL_BIN=$(TARGET_DIR)/ipl.bin
OS_BIN=$(TARGET_DIR)/haribote.sys
TARGET_IMG=$(TARGET_DIR)/haribote.img

#debug
LIST_IPL=$(TARGET_DIR)/ipl.lst
LIST_OS=$(TARGET_DIR)/os.lst

QEMU=qemu-system-x86_64

all: $(TARGET_IMG)

$(OS_BIN): $(OS_SRC) $(OS_LINK_SCRIPT)
	mkdir -p $(TARGET_DIR)
	gcc -nostdlib -o $@ -T$(OS_LINK_SCRIPT) $(OS_SRC)
	gcc -T $(OS_LINK_SCRIPT) -c -g -Wa,-a,-ad $(OS_SRC) -o bin/os.o > $(LIST_OS)

$(IPL_BIN): $(IPL_SRC) $(IPL_LINK_SCRIPT)
	mkdir -p $(TARGET_DIR)
	gcc -nostdlib -o $@ -T$(IPL_LINK_SCRIPT) $(IPL_SRC)
	gcc -T $(IPL_LINK_SCRIPT) -c -g -Wa,-a,-ad $(IPL_SRC) -o bin/ipl.o > $(LIST_IPL)

$(TARGET_IMG): $(OS_BIN) $(IPL_BIN)
	#イメージ作成、IPLをブートセクタに配置
	mformat -f 1440 -B $(IPL_BIN) -C -i $(TARGET_IMG) ::
	#OSのプログラムをイメージにコピーする
	mcopy $(OS_BIN) -i $(TARGET_IMG) ::

run: all
	$(QEMU) -m 32 -drive format=raw,file=$(TARGET_IMG),if=floppy
	#hexdump $(TARGET_IMG)

debug:all
	$(QEMU) -drive format=raw,file=$(TARGET_IMG),if=floppy -gdb tcp::10000 -S

clean:
	rm -rf $(TARGET_DIR)

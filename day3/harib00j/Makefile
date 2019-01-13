IPL_LINK_SCRIPT=ipl.ls
OS_LINK_SCRIPT=os.ls

IPL_SRC=ipl10.s
OS_SRC=asmhead.s
BOOTPACK_SRC=bootpack.c
ASM_LIB_SRC=asm_func.s

TARGET_DIR=bin
IPL_BIN=$(TARGET_DIR)/ipl.bin
OS_BIN=$(TARGET_DIR)/asmhead.bin
BOOTPACK_BIN=$(TARGET_DIR)/bootpack.bin
ASM_LIB_BIN=$(TARGET_DIR)/asm_func.o

SYSTEM_IMG=$(TARGET_DIR)/haribote.sys

TARGET_IMG=$(TARGET_DIR)/haribote.img

#debug
LIST_IPL=$(TARGET_DIR)/ipl.lst
LIST_OS=$(TARGET_DIR)/os.lst
LIST_ASM_LIB=$(TARGET_DIR)/asm_func.lst

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

$(BOOTPACK_BIN): $(BOOTPACK_SRC) $(ASM_LIB_BIN)
	mkdir -p $(TARGET_DIR)
	
	gcc -nostdlib -m32 -c -o bin/bootpack.o $(BOOTPACK_SRC)
	ld -m elf_i386 -o $@ -T bootpack.ls -e HariMain --oformat=binary bin/bootpack.o $(ASM_LIB_BIN)
	#gcc -nostdlib -m32 -e HariMain -T bootpack.ls -o $@ bin/bootpack.o $(ASM_LIB_BIN)
	
	#ボツ
	#gcc -nostdlib -m32 -T bootpack.ls -o $@ $(BOOTPACK_SRC)
	
	#以下ボツ
	#gcc -nostdlib -m32 -c -o bin/bootpack.o $(BOOTPACK_SRC)
	#objcopy -O binary -j .text bin/bootpack.o $@
	#ld -m elf_i386 -o $@ -e HariMain --oformat=binary bin/bootpack.o

$(ASM_LIB_BIN): $(ASM_LIB_SRC)
	mkdir -p $(TARGET_DIR)
	gcc -m32 -c -g -Wa,-a,-ad $(ASM_LIB_SRC) -o $(ASM_LIB_BIN) > $(LIST_ASM_LIB)

$(SYSTEM_IMG): $(OS_BIN) $(BOOTPACK_BIN)
	cat $(OS_BIN) $(BOOTPACK_BIN) > $@

$(TARGET_IMG): $(SYSTEM_IMG) $(IPL_BIN)
	#イメージ作成、IPLをブートセクタに配置
	mformat -f 1440 -B $(IPL_BIN) -C -i $(TARGET_IMG) ::
	#OSのプログラムをイメージにコピーする
	mcopy $(SYSTEM_IMG) -i $(TARGET_IMG) ::

run: all
	$(QEMU) -m 32 -drive format=raw,file=$(TARGET_IMG),if=floppy

debug:all
	$(QEMU) -drive format=raw,file=$(TARGET_IMG),if=floppy -gdb tcp::10000 -S

clean:
	rm -rf $(TARGET_DIR)

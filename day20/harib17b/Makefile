#Makefile

CC = gcc
CFLAGS = -nostdlib -m32 -fno-builtin -O0
INCLUDE = -I include -I ../../tools/stdlibc/include

LD = ld
LFLAGS = -m elf_i386

QEMU = qemu-system-x86_64

TARGET_DIR = bin
LST_DIR = $(TARGET_DIR)/lst
TMP_DIR = $(TARGET_DIR)/tmp

OS_SRC_DIR = src
OS_SRC=$(wildcard $(OS_SRC_DIR)/*.c)
OS_LS = scripts/bootpack.lds
OS = $(TARGET_DIR)/os.bin

OS_MMAP = $(TARGET_DIR)/os.map

OS_ENTRY_POINT = HariMain

SYSTEM_IMG = bin/haribote.bin

ASMLIB_SRC = src/asm/asm_func.s
ASMLIB = $(TARGET_DIR)/asm_func.o

IPL_SRC = src/asm/boot/ipl10.s
IPL_LS = scripts/ipl.lds
IPL = $(TARGET_DIR)/ipl10.bin

OSL_SRC = src/asm/boot/asmhead.s
OSL_LS = scripts/asmhead.lds
OSL = $(TARGET_DIR)/asmhead.bin

#external lib
STDLIBC_DIR = ../../tools/stdlibc
STDLIBC = $(STDLIBC_DIR)/bin/stdlibc.o

FONT_DIR = ../../tools/makefont
FONT = $(FONT_DIR)/bin/hankaku.o

EXCLUDE_EXLIB_DEP_FILE = *.swp

IMG = $(TARGET_DIR)/haribote.img

HLT_APP = $(TARGET_DIR)/hlt.hrb

all: $(IMG)

$(FONT) : $(shell find $(FONT_DIR) -type f -not -name '$(EXCLUDE_EXLIB_DEP_FILE)' -not -name '$(notdir $(FONT))')
	@echo [Dependent Files] FONT:  $^
	cd $(FONT_DIR); make

$(STDLIBC) : $(shell find $(STDLIBC_DIR) -type f -not -name '$(EXCLUDE_EXLIB_DEP_FILE)' -not -name '$(notdir $(STDLIBC))')
	@echo [Dependent Files] STDLIBC: $^
	cd $(STDLIBC_DIR); make all

$(IMG): $(IPL) $(OSL) $(OS) $(HLT_APP)
	cat $(OSL) $(OS) > $(SYSTEM_IMG)
	mformat -f 1440 -B $(IPL) -C -i $(IMG) ::
	mcopy $(SYSTEM_IMG) -i $(IMG) ::
	mcopy $(HLT_APP) -i $(IMG) ::
	mcopy Makefile -i $(IMG) ::

$(OS): $(addprefix $(TARGET_DIR)/, $(notdir $(OS_SRC:.c=.o))) $(STDLIBC) $(ASMLIB) $(FONT)
	ld $(LFLAGS) -o $@ -T $(OS_LS) -e HariMain --oformat=binary -Map=$(OS_MMAP) $^

$(ASMLIB): $(ASMLIB_SRC)
	$(CC) $(CFLAGS) -c -g -Wa,-a,-ad $< -o $@ > $(addprefix $(LST_DIR)/, $(notdir $(@F:.o=.lst)))

$(TARGET_DIR)/%.o : $(OS_SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDE) -nostdlib -m32 -c -o $@ $<

$(IPL): $(IPL_SRC)
	mkdir -p $(TARGET_DIR)
	mkdir -p $(LST_DIR)
	mkdir -p $(TMP_DIR)
	gcc $(CFLAGS) -o $@ -T$(IPL_LS) $(IPL_SRC)

$(OSL): $(OSL_SRC)
	$(CC) $(CFLAGS) -o $@ -T $(OSL_LS) $(OSL_SRC)
	$(CC) $(CFLAGS) -o $(addprefix $(TMP_DIR)/, $(notdir $(@F:.s=.o))) -T $(OSL_LS) -c -g -Wa,-a,-ad $(OSL_SRC) > $(addprefix $(LST_DIR)/, $(notdir $(@F:.bin=.lst)))

$(HLT_APP): src/hlt_app/hlt.s
	gcc -c $(CFLAGS) -m32 -o $(TARGET_DIR)/hlt.o src/hlt_app/hlt.s
	objcopy -O binary $(TARGET_DIR)/hlt.o $(HLT_APP)

run: all
	$(QEMU) -m 32M -drive format=raw,file=$(IMG),if=floppy

debug: all
	$(QEMU) -drive format=raw,file=$(IMG),if=floppy -gdb tcp::10000 -S

clean:
	rm -rf $(TARGET_DIR)
	cd $(FONT_DIR); make clean
	cd $(STDLIBC_DIR); make clean

#Makefile

CC = gcc
CWARN=-Wall -Wno-pointer-sign -Wno-unused-variable
CFLAGS = -nostdlib -m32 -fno-builtin -O0 $(CWARN)
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

IPL_SRC = src/asm/boot/ipl20.s
IPL_LS = scripts/ipl.lds
IPL = $(TARGET_DIR)/ipl20.bin

OSL_SRC = src/asm/boot/asmhead.s
OSL_LS = scripts/asmhead.lds
OSL = $(TARGET_DIR)/asmhead.bin

#external lib
STDLIBC_DIR = ../../tools/stdlibc
STDLIBC = $(STDLIBC_DIR)/bin/stdlibc.o

FONT_DIR = ../../tools/makefont
FONT = $(FONT_DIR)/bin/hankaku.o

JPFONT = ../../tools/font_jpn/nihongo.fnt
JPFONT_TEST_FILE = ../../tools/font_jpn/sjis.txt ../../tools/font_jpn/euc.txt

EXCLUDE_EXLIB_DEP_FILE = *.swp

IMG = $(TARGET_DIR)/haribote.img

#application
APP_TARGET_DIR = $(TARGET_DIR)/app/dist
APP_SRC_DIR = src/app
APP_SRC = $(shell find $(APP_SRC_DIR) -type f)
APP_DIRS = $(shell find $(APP_SRC_DIR) -maxdepth 1 -type d -not -path $(APP_SRC_DIR))

PICT1 = ../../tools/pictdata/fujisan.jpg
PICT2 = ../../tools/pictdata/night.bmp
MML1 = ../../tools/mmldata/daigo.mml
MML2 = ../../tools/mmldata/daiku.mml
MML3 = ../../tools/mmldata/fujisan.mml
MML4 = ../../tools/mmldata/kirakira.mml

define make_app
	cd $1; make;

endef

define copy_app
	mcopy $1 -i $(IMG) ::

endef

all: $(IMG)

apps : $(APP_SRC)
	@echo [Application] build
	$(foreach x, $(APP_DIRS), $(call make_app, $(x)))

$(FONT) : $(shell find $(FONT_DIR) -type f -not -name '$(EXCLUDE_EXLIB_DEP_FILE)' -not -name '$(notdir $(FONT))')
	@echo [Dependent Files] FONT:  $^
	cd $(FONT_DIR); make

$(STDLIBC) : $(shell find $(STDLIBC_DIR) -type f -not -name '$(EXCLUDE_EXLIB_DEP_FILE)' -not -name '$(notdir $(STDLIBC))')
	@echo [Dependent Files] STDLIBC: $^
	cd $(STDLIBC_DIR); make all

$(IMG): $(IPL) $(OSL) $(OS) apps $(JPFONT) $(JPFONT_TEST_FILE)
	cat $(OSL) $(OS) > $(SYSTEM_IMG)
	mformat -f 1440 -B $(IPL) -C -i $(IMG) ::
	mcopy $(SYSTEM_IMG) -i $(IMG) ::
	$(foreach x, $(shell find $(APP_TARGET_DIR) -type f), $(call copy_app, $(x)))
	mcopy $(IPL_SRC) -i $(IMG) ::
	mcopy $(JPFONT) -i $(IMG) ::
	mcopy $(JPFONT_TEST_FILE) -i $(IMG) ::
	mcopy $(PICT1) -i $(IMG) ::
	mcopy $(PICT2) -i $(IMG) ::
	mcopy $(MML1) -i $(IMG) ::
	mcopy $(MML2) -i $(IMG) ::
	mcopy $(MML3) -i $(IMG) ::
	mcopy $(MML4) -i $(IMG) ::

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

run: all
	$(QEMU) -m 32M -drive format=raw,file=$(IMG),if=floppy

debug: all
	$(QEMU) -drive format=raw,file=$(IMG),if=floppy -gdb tcp::10000 -S

clean:
	rm -rf $(TARGET_DIR)
	cd $(FONT_DIR); make clean
	cd $(STDLIBC_DIR); make clean

#Makefile

CC = gcc
CFLAGS = -nostdlib -m32 -fno-builtin -O0 -fno-common

LFLAGS = -m elf_i386

LIB = $(TARGET_DIR)/lib/apilib.lib
LS = ../../../scripts/app_super_large_stack.lds

STDLIB_DIR = ../../../../../tools/stdlibc
STDLIB = $(STDLIB_DIR)/bin/stdlibc.o

TARGET_DIR = ../../../bin/app
DIST_DIR = $(TARGET_DIR)/dist
TMP_DIR = $(TARGET_DIR)/tmp/gview

APP = $(DIST_DIR)/gview.hrb
APP_SRC1 = gview.c
APP_SRC2 = jpeg.c
APP_SRC_ASM = bmp.nasm

INCLUDE = -I ../lib/include -I $(STDLIB_DIR)/include

$(APP): $(APP_SRC) $(LIB)
	mkdir -p $(DIST_DIR)
	mkdir -p $(TMP_DIR)
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $(TMP_DIR)/$(APP_SRC1:.c=.o) $(APP_SRC1)
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $(TMP_DIR)/$(APP_SRC2:.c=.o) $(APP_SRC2)
	nasm -f elf -o $(TMP_DIR)/$(APP_SRC_ASM:.nasm=.o) $(APP_SRC_ASM)
	ld $(LFLAGS) -o $(APP) -e HariMain -T $(LS) $(STDLIB) $(TMP_DIR)/$(APP_SRC1:.c=.o) $(TMP_DIR)/$(APP_SRC2:.c=.o) $(TMP_DIR)/$(APP_SRC_ASM:.nasm=.o) $(LIB)

$(LIB):
	cd ../lib; make

$(STDLIB):
	cd $(STDLIB_DIR); make

clean:
	rm -rf $(TMP_DIR)
	rm -f $(APP)

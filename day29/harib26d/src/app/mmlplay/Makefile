#Makefile

CC = gcc
CFLAGS = -nostdlib -m32 -fno-builtin -O0 -fno-common
INCLUDE = -I ../lib/include -I $(STDLIB_DIR)/include

LFLAGS = -m elf_i386

LIB = $(TARGET_DIR)/lib/apilib.lib
LS = ../../../scripts/app_super_large_stack.lds

STDLIB_DIR = ../../../../../tools/stdlibc
STDLIB = $(STDLIB_DIR)/bin/stdlibc.o

TARGET_DIR = ../../../bin/app
DIST_DIR = $(TARGET_DIR)/dist
TMP_DIR = $(TARGET_DIR)/tmp/mmlplay

APP = $(DIST_DIR)/mmlplay.hrb
APP_SRC = mmlplay.c

$(APP): $(APP_SRC) $(LIB)
	mkdir -p $(DIST_DIR)
	mkdir -p $(TMP_DIR)
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $(TMP_DIR)/$(APP_SRC:.c=.o) $(APP_SRC)
	ld $(LFLAGS) -o $(APP) -e HariMain -T $(LS) $(STDLIB) $(TMP_DIR)/$(APP_SRC:.c=.o) $(LIB)

$(LIB):
	cd ../lib; make

clean:
	rm -rf $(TMP_DIR)
	rm -f $(APP)

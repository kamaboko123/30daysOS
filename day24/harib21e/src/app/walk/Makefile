#Makefile

CC = gcc
CFLAGS = -nostdlib -m32 -fno-builtin -O0 -fno-common

LFLAGS = -m elf_i386

LIB = $(TARGET_DIR)/lib/libhrb
LS = ../../../scripts/app.lds

STDLIB_DIR = ../../../../../tools/stdlibc
STDLIB = $(STDLIB_DIR)/bin/stdlibc.o

TARGET_DIR = ../../../bin/app
DIST_DIR = $(TARGET_DIR)/dist
TMP_DIR = $(TARGET_DIR)/tmp/walk

APP = $(DIST_DIR)/walk
APP_SRC = walk.c

INCLUDE = -I ../lib/include -I $(STDLIB_DIR)/include

$(APP): $(APP_SRC) $(LIB)
	mkdir -p $(DIST_DIR)
	mkdir -p $(TMP_DIR)
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $(TMP_DIR)/$(APP_SRC:.c=.o) $(APP_SRC)
	ld $(LFLAGS) -o $(APP) -e __start -T $(LS) $(LIB) $(STDLIB) $(TMP_DIR)/$(APP_SRC:.c=.o)

$(LIB):
	cd ../lib; make

$(STDLIB):
	cd $(STDLIB_DIR); make

clean:
	rm -rf $(TMP_DIR)
	rm -f $(APP)

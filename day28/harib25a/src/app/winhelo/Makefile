#Makefile

CC = gcc
CFLAGS = -nostdlib -m32 -fno-builtin -O0
INCLUDE = -I ../lib/include

LFLAGS = -m elf_i386

LIB = $(TARGET_DIR)/lib/apilib.lib
LS = ../../../scripts/app.lds

TARGET_DIR = ../../../bin/app
DIST_DIR = $(TARGET_DIR)/dist
TMP_DIR = $(TARGET_DIR)/tmp/winhelo

APP = $(DIST_DIR)/winhelo.hrb
APP_SRC = winhelo.c

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

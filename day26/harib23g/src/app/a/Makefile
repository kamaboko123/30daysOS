#Makefile

CC = gcc
CFLAGS = -nostdlib -m32 -fno-builtin -O0
INCLUDE = -I ../lib/include

LFLAGS = -m elf_i386

LIB = $(TARGET_DIR)/lib/libhrb
LS = ../../../scripts/app.lds

TARGET_DIR = ../../../bin/app
DIST_DIR = $(TARGET_DIR)/dist
TMP_DIR = $(TARGET_DIR)/tmp/a

APP = $(DIST_DIR)/a
APP_SRC = a.c

$(APP): $(APP_SRC) $(LIB)
	mkdir -p $(DIST_DIR)
	mkdir -p $(TMP_DIR)
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $(TMP_DIR)/$(APP_SRC:.c=.o) $(APP_SRC)
	ld $(LFLAGS) -o $(APP) --oformat=binary -e __start -T $(LS) $(LIB) $(TMP_DIR)/$(APP_SRC:.c=.o)

$(LIB):
	cd ../lib; make

clean:
	rm -rf $(TMP_DIR)
	rm -f $(APP)

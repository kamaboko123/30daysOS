#Makefile

CC = gcc
CFLAGS = -nostdlib -m32 -fno-builtin -O0
LFLAGS = -m elf_i386

TARGET_DIR = ../../../bin/app
DIST_DIR = $(TARGET_DIR)/dist
TMP_DIR = $(TARGET_DIR)/tmp/crack7
APP = $(DIST_DIR)/crack7.hrb
APP_SRC = crack7.s

LIB = $(TARGET_DIR)/lib/apilib.lib
LS = ../../../scripts/app.lds

$(APP): $(APP_SRC) $(LIB)
	mkdir -p $(DIST_DIR)
	mkdir -p $(TMP_DIR)
	$(CC) -c $(CFLAGS) -o $(TMP_DIR)/$(APP_SRC:.s=.o) $(APP_SRC)
	ld $(LFLAGS) -o $(APP) --oformat=binary -e HariMain -T $(LS) $(TMP_DIR)/$(APP_SRC:.s=.o) $(LIB)

$(LIB):
	cd ../lib; make

clean:
	rm -rf $(TMP_DIR)
	rm -f $(APP)

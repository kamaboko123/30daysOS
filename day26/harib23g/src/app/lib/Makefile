#Makefile

CC = gcc
CFLAGS = -nostdlib -m32 -fno-builtin -O0
INCLUDE = -I include -I ../../tools/stdlibc/include

TARGET_DIR = ../../../bin/app
DIST_DIR = $(TARGET_DIR)/lib
TMP_DIR = $(TARGET_DIR)/tmp/lib
APP = $(DIST_DIR)/libhrb
APP_SRC = haribote.s

$(APP): $(APP_SRC)
	mkdir -p $(DIST_DIR)
	mkdir -p $(TMP_DIR)
	$(CC) -c $(CFLAGS) -o $(APP) $(APP_SRC)

clean:
	rm -rf $(TMP_DIR)
	rm -f $(APP)

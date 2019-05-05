#Makefile

CC = gcc
CFLAGS = -nostdlib -m32 -fno-builtin -O0
LS = ../../../scripts/app.lds

TARGET_DIR = ../../../bin/app
DIST_DIR = $(TARGET_DIR)/dist
TMP_DIR = $(TARGET_DIR)/tmp/hello5
APP = $(DIST_DIR)/hello5.hrb
APP_SRC = hello5.s

$(APP): $(APP_SRC)
	mkdir -p $(DIST_DIR)
	mkdir -p $(TMP_DIR)
	$(CC) $(CFLAGS) -T $(LS) -o $(APP) $(APP_SRC)

clean:
	rm -rf $(TMP_DIR)
	rm -f $(APP)

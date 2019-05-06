#Makefile

CC = gcc
CFLAGS = -nostdlib -m32 -fno-builtin -O0
INCLUDE = -I include -I ../../tools/stdlibc/include

ARCHIVE = ar

TARGET_DIR = ../../../bin/app
DIST_DIR = $(TARGET_DIR)/lib
TMP_DIR = $(TARGET_DIR)/tmp/lib
TARGET = $(DIST_DIR)/apilib.lib

SRC_DIR = src
SRC = $(shell find $(SRC_DIR) -type f | grep -v "\/\.")
OBJ = $(addprefix $(TMP_DIR)/, $(notdir $(SRC:.s=.o)))

all: $(TARGET)


$(TARGET): $(OBJ)
	mkdir -p $(DIST_DIR)
	$(ARCHIVE) r $@ $^

$(TMP_DIR)/%.o : $(SRC_DIR)/%.s
	mkdir -p $(TMP_DIR)
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -rf $(TMP_DIR)
	rm -rf $(DIST_DIR)

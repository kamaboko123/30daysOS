CC = gcc
CFLAGS = -nostdlib -m32 -fno-builtin -g
INCLUDE = -I include

TARGET_DIR = bin
SRC_DIR = src

SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(addprefix $(TARGET_DIR)/, $(notdir $(SRC:.c=.o)))

all: $(OBJ)

$(TARGET_DIR)/%.o : $(SRC_DIR)/%.c
	mkdir -p $(TARGET_DIR)
	$(CC) $(INCLUDE) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(TARGET_DIR)


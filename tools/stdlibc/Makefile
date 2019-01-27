CC = gcc
CFLAGS = -nostdlib -m32 -fno-builtin -g
INCLUDE = -I include

TARGET_DIR = bin
SRC_DIR = src

SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(addprefix $(TARGET_DIR)/, $(notdir $(SRC:.c=.o)))

TEST_MAIN = $(TARGET_DIR)/test_main.o
TEST_MAIN_SRC = $(SRC_DIR)/test/main.c

TEST = $(TARGET_DIR)/test_stdlibc

all: $(OBJ)

$(TARGET_DIR)/%.o : $(SRC_DIR)/%.c
	mkdir -p $(TARGET_DIR)
	$(CC) $(INCLUDE) $(CFLAGS) -c -o $@ $<

test: $(OBJ) $(TEST_MAIN_SRC)
	$(CC) $(INCLUDE) $(CFLAGS) -c -o $(TEST_MAIN) $(TEST_MAIN_SRC)
	$(CC) -m32 $(INCLUDE) -o $(TEST) $(TEST_MAIN) $(OBJ) -lcunit

clean:
	rm -rf $(TARGET_DIR)


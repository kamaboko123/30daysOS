CC = gcc
CFLAGS = -g

MAKEFONT_SRC = makefont.c
HANKAKU_SRC = hankaku.txt

TARGET_DIR = bin
MAKEFONT_BIN = $(TARGET_DIR)/makefont
HANKAKU_ASM = $(TARGET_DIR)/hankaku.s
TARGET_FONT_OBJ = $(TARGET_DIR)/hankaku.o

$(TARGET_FONT_OBJ) : $(HANKAKU_ASM)
	mkdir -p $(TARGET_DIR)
	gcc -m32 -c $(HANKAKU_ASM) -o $@

$(HANKAKU_ASM) : $(MAKEFONT_BIN)
	$(MAKEFONT_BIN) $(HANKAKU_SRC) > $(HANKAKU_ASM)

$(MAKEFONT_BIN): $(MAKEFONT_SRC)
	mkdir -p $(TARGET_DIR)
	$(CC) $(CFLAGS) -o $@ $(MAKEFONT_SRC)

clean:
	rm -rf $(TARGET_DIR)

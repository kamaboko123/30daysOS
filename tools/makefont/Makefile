CC = gcc
CFLAGS = -g

MAKEFONT_SRC = makefont.c
HANKAKU_SRC = hankaku.txt

TARGET_DIR = bin
MAKEFONT_BIN = $(TARGET_DIR)/makefont
HANKAKU_ASM = hankaku.s

hankaku : $(MAKEFONT_BIN)
	$(MAKEFONT_BIN) $(HANKAKU_SRC) > $(HANKAKU_ASM)

$(MAKEFONT_BIN): $(MAKEFONT_SRC)
	mkdir -p $(TARGET_DIR)
	$(CC) $(CFLAGS) -o $@ $(MAKEFONT_SRC)

clean:
	rm -rf $(TARGET_DIR)

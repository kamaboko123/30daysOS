.text
.code16

movb $0x13, %al
movb $0x00, %ah
int $0x10

fin:
    hlt
    jmp fin


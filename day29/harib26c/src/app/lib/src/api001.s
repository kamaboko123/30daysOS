.arch i486

.global api_putchar
#void api_putchar(int c)
api_putchar:
    movl $1, %edx
    movb 4(%esp), %al
    int $0x40
    ret


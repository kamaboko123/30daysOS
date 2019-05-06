.arch i486

#void api_closewin(int win)
.global api_closewin
api_closewin:
    push %ebx
    mov $14, %edx
    movl 8(%esp), %ebx #win
    int $0x40
    pop %ebx



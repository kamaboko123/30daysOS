.arch i486

.global api_putstr1
#void api_putstr1(char *s, int l)
    push %ebx
    movl $3, %edx
    movl 8(%esp), %ebx
    movl 12(%esp), %ecx
    int $0x40
    pop %ebx
    ret

.arch i486

#void api_free(char *addr, int size)
.global api_free
api_free:
    push %ebx
    movl $10, %edx
    movl %cs:(0x0020), %ebx
    movl 8(%esp), %eax #addr
    movl 12(%esp), %ecx #size
    int $0x40
    pop %ebx
    ret



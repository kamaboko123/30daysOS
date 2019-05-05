.arch i486

#char *api_malloc(int size)
.global api_malloc
api_malloc:
    push %ebx
    movl $9, %edx
    movl %cs:(0x0020), %ebx
    movl 8(%esp), %ecx #size
    int $0x40
    pop %ebx
    ret



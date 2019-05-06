.arch i486

#void api_fseek(int fhandle, int offset, int mode)
.global api_fseek
api_fseek:
    push %ebx
    movl $23, %edx
    movl 8(%esp), %eax #fhandle
    movl 12(%esp), %ecx #offset
    movl 16(%esp), %ebx #mode
    int $0x40
    pop %ebx
    ret


.arch i486

#int api_fread(char *buf, int maxsize, int fhandle)
.global api_fread
api_fread:
    push %ebx
    movl $25, %edx
    movl 16(%esp), %eax
    movl 12(%esp), %ecx
    movl 8(%esp), %ebx
    int $0x40
    pop %ebx
    ret


.arch i486

#int api_cmdline(char *buf, int maxsize)
.global api_cmdline
api_cmdline:
    push %ebx
    movl $26, %edx
    movl 12(%esp), %ecx #maxsize
    movl 8(%esp), %ebx #buf
    int $0x40
    pop %ebx
    ret


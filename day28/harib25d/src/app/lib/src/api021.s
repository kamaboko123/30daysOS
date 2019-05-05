.arch i486

#int api_fopen(char *fname)
.global api_fopen
api_fopen:
    push %ebx
    movl $21, %edx
    movl 8(%esp), %ebx #fname
    int $0x40
    pop %ebx
    ret


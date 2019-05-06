.arch i486

#int api_fsize(int fhandle, int mode)
.global api_fsize
api_fsize:
    movl $24, %edx
    movl 4(%esp), %eax #fhandle
    movl 8(%esp), %ecx #fhandle
    int $0x40
    ret


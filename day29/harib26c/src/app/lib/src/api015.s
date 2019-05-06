.arch i486

#int api_getkey(int mode)
.global api_getkey
api_getkey:
    movl $15, %edx
    movl 4(%esp), %eax #mode
    int $0x40
    ret



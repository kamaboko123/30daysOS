.arch i486

#int api_getlang(void)
.global api_getlang
api_getlang:
    movl $27, %edx
    int $0x40
    ret


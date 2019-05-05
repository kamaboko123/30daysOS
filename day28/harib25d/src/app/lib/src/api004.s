.arch i486

#void api_end(void)
.global api_end
api_end:
    movl $4, %edx
    int $0x40


.arch i486

#void api_beep(int tone)
.global api_beep
api_beep:
    movl $20, %edx
    movl 4(%esp), %eax
    int $0x40
    ret


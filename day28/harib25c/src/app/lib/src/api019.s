.arch i486

#void api_freetimer(int timer)
.global api_freetimer
api_freetimer:
    push %ebx
    movl $19, %edx
    movl 8(%esp), %ebx  #imer
    int $0x40
    pop %ebx
    ret



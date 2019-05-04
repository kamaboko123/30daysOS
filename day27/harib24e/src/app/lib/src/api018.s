.arch i486

#void api_settimer(int timer, int time)
.global api_settimer
api_settimer:
    push %ebx
    movl $18, %edx
    movl 8(%esp), %ebx  #imer
    movl 12(%esp), %eax #time
    int $0x40
    pop %ebx
    ret



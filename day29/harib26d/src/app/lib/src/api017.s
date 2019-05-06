.arch i486

#void api_inittimer(int timer, int data)
.global api_inittimer
api_inittimer:
    push %ebx
    movl $17, %edx
    movl 8(%esp), %ebx  #timer
    movl 12(%esp), %eax #data
    int $0x40
    pop %ebx
    ret


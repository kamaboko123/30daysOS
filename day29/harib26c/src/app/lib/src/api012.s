.arch i486

#void api_refreshwin(int win, int x0, int y0, int x1, int y1)
.global api_refreshwin
api_refreshwin:
    push %edi
    push %esi
    push %ebx
    movl $12, %edx
    movl 16(%esp), %ebx #win
    movl 20(%esp), %eax #x0
    movl 24(%esp), %ecx #y0
    movl 28(%esp), %esi #x1
    movl 32(%esp), %edi #y1
    int $0x40
    pop %ebx
    pop %esi
    pop %edi
    ret



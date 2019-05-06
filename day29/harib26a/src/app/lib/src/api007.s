.arch i486

#void boxfillwin(int win, int x0, int y0, int x1, int y1, int col)
.global api_boxfillwin
api_boxfillwin:
    push %edi
    push %esi
    push %ebp
    push %ebx
    movl $7, %edx
    movl 20(%esp), %ebx #win
    movl 24(%esp), %eax #x0
    movl 28(%esp), %ecx #y0
    movl 32(%esp), %esi #x1
    movl 36(%esp), %edi #y1
    movl 40(%esp), %ebp #str
    int $0x40
    pop %ebx
    pop %ebp
    pop %esi
    pop %edi
    ret



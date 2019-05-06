.arch i486

#void putstrwin(int win, int x, int y, int col, int len, char *str)
.global api_putstrwin
api_putstrwin:
    push %edi
    push %esi
    push %ebp
    push %ebx
    movl $6, %edx
    movl 20(%esp), %ebx #win
    movl 24(%esp), %esi #x
    movl 28(%esp), %edi #y
    movl 32(%esp), %eax #col
    movl 36(%esp), %ecx #len
    movl 40(%esp), %ebp #str
    int $0x40
    pop %ebx
    pop %ebp
    pop %esi
    pop %edi
    ret



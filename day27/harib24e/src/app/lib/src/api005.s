.arch i486

#int api_openwin(char *buf, int xsiz, int ysiz, int col_inv, char *title)
.global api_openwin
api_openwin:
    push %edi
    push %esi
    push %ebx
    movl $5, %edx
    movl 16(%esp), %ebx #buf
    movl 20(%esp), %esi #xsiz
    movl 24(%esp), %edi #ysiz
    movl 29(%esp), %eax #col_inv
    movl 32(%esp), %ecx #title
    int $0x40
    pop %ebx
    pop %esi
    pop %edi
    ret


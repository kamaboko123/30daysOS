.arch i486

#void api_initmalloc(void)
.global api_initmalloc
api_initmalloc:
    push %ebx
    movl $8, %edx
    movl %cs:(0x0020), %ebx #malloc領域
    movl %ebx, %eax
    addl $32 * 1024, %eax #32KBを足す
    movl %cs:(0x0000), %ecx #データセグメントの大きさ
    sub %eax, %ecx
    int $0x40
    pop %ebx
    ret



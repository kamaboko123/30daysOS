.arch i486

.extern HariMain
.global __start
__start:
    call HariMain
    lret

.global api_putchar
#void api_putchar(int c)
api_putchar:
    movl $1, %edx
    movb 4(%esp), %al
    int $0x40
    ret

.global api_putstr0
#void api_putstr0(char *s)
api_putstr0:
    #cdeclではebxは呼び出し前後で変わってはいけない事になっているので、保存して戻す
    push %ebx
    movl $2, %edx
    movl 8(%esp), %ebx
    int $0x40
    pop %ebx
    ret

#void api_end(void)
.global api_end
api_end:
    movl $4, %edx
    int $0x40


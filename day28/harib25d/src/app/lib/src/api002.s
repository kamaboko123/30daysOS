.arch i486

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


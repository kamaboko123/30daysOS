.arch i486

.global api_putchar


api_putchar:
    mov $1, %edx
    mov 4(%esp), %al
    int $0x40
    ret

.extern HariMain
.global __start
__start:
    call HariMain
    lret

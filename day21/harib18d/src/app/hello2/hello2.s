.arch i486

movl $2, %edx
movl $msg, %ebx
int $0x40
lret

msg:
    .string "hello"


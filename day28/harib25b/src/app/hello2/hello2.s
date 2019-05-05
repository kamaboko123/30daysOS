.arch i486

movl $2, %edx
movl $msg, %ebx
int $0x40

#終了
mov $4, %edx
int $0x40

msg:
    .string "hello"


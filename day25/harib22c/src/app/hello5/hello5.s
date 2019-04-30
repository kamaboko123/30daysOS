.arch i486

.text

.global HariMain
HariMain:
    movl $2, %edx
    movl $msg, %ebx
    int $0x40
    movl $4, %edx
    int $0x40

.data
msg:
    .string "hello, world"
    .byte 0x0a, 0x00


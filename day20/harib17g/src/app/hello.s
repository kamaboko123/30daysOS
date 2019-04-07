.arch i486

movl $msg, %ecx

putloop:
    movb %cs:(%ecx), %al
    cmpb $0, %al
    je fin
    int $0x40
    addl $1, %ecx
    jmp putloop

fin:
    lret

msg:
    .string "hello"


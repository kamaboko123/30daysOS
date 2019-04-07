.arch i486

movb $0x41, %al
lcall $2*8, $0x4c1f

fin:
    hlt
    jmp fin


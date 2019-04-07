.arch i486

movb 0x41, %al
call 0x4c1f

fin:
    hlt
    jmp fin


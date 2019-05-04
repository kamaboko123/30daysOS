.arch i486

#int api_alloctimer(void)
.global api_alloctimer
api_alloctimer:
    movl $16, %edx
    int $0x40
    ret



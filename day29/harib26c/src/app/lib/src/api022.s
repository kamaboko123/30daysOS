.arch i486

#void api_fclose(int fhandle)
.global api_fclose
api_fclose:
    movl $22, %edx
    movl 4(%esp), %eax #fhandle
    int $0x40
    ret


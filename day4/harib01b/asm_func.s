.arch i486

.text

#void io_htl(void)
.global io_hlt

#void write_mem8(int addr, int data)
.global write_mem8

io_hlt:
    hlt
    ret

write_mem8:
    movl 4(%esp), %ecx
    movb 8(%esp), %al
    mov %al, (%ecx)
    ret


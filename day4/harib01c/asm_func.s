.arch i486

.text

#void io_htl(void)
.global io_hlt

io_hlt:
    hlt
    ret


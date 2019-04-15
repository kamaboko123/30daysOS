.arch i486

movb $0x34, %al
outb %al, $0x43
movb $0xff, %al
outb %al, $0x40
movb $0xff, %al
outb %al, $0x40

/*
io_out8(PIT_CTRL, 0x34)
io_out8(PIT_CNT0, 0xff)
io_out8(PIT_CNT0, 0xff)
*/

mov $4, %edx
int $0x40


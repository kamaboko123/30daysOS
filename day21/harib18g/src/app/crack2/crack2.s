.arch i486

movl $1 * 8, %eax
movw %ax, %ds
movb $0, (0x102600)

mov $4, %edx
int $0x40


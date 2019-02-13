.arch i486

.text

.global io_hlt, io_cli, io_sti, io_stihlt
.global io_in8, io_in16, io_in32
.global io_out8, io_out16, io_out32
.global io_load_eflags, io_store_eflags
.global load_gdtr, load_idtr

#void io_htl(void)
io_hlt:
    hlt
    ret

#void io_cli(void)
io_cli:
    cli
    ret

#void io_sti(void)
io_sti:
    sti
    ret

#void io_stihlt(void)
io_stihlt:
    sti
    hlt
    ret

#int io_in8(int port)
io_in8:
    movl 4(%esp), %edx
    movl $0, %eax
    inb %dx, %al
    ret

#int io_in16(int port)
io_in16:
    movl 4(%esp), %edx
    movl $0, %eax
    inw %dx, %ax
    ret

#int io_in32(int port)
io_in32:
    movl 4(%esp), %edx
    inl %dx, %eax
    ret

#void io_out8(int port, int data)
io_out8:
    movl 4(%esp), %edx
    movb 8(%esp), %al
    outb %al, %dx
    ret

#void io_out16(int port, int data)
io_out16:
    movl 4(%esp), %edx
    movl 8(%esp), %eax
    outw %ax, %dx
    ret

#void io_out32(int port, int data)
io_out32:
    movl 4(%esp), %edx
    movl 8(%esp), %eax
    outl %eax, %dx
    ret

#int io_load_eflags(void)
io_load_eflags:
    pushf
    pop %eax
    ret

#int io_store_eflags(void)
io_store_eflags:
    movl 4(%esp), %eax
    push %eax
    popf
    ret

#void load_gdtr(int limit, int addr)
load_gdtr:
    mov 4(%esp), %ax #limit
    mov %ax, 6(%esp)
    lgdt 6(%esp)
    ret

#void load_idtr(int limti, int addr)
load_idtr:
    mov 4(%esp), %ax #limit
    mov %ax, 6(%esp)
    lidt 6(%esp)
    ret

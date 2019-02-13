.arch i486

.text

.global io_hlt, io_cli, io_sti, io_stihlt
.global io_in8, io_in16, io_in32
.global io_out8, io_out16, io_out32
.global io_load_eflags, io_store_eflags
.global load_gdtr, load_idtr
.global load_cr0, store_cr0
.global asm_inthandler21, asm_inthandler2c, asm_inthandler27

.extern inthandler21, inthandler2c, inthandler27

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
    movw 4(%esp), %ax #limit
    movw %ax, 6(%esp)
    lgdt 6(%esp)
    ret

#void load_idtr(int limit, int addr)
load_idtr:
    movw 4(%esp), %ax #limit
    movw %ax, 6(%esp)
    lidt 6(%esp)
    ret

#int io_load_cr0(void)
load_cr0:
    movl %cr0, %eax
    ret

#void io_store_cr0(int cr0)
store_cr0:
    movl 4(%esp), %eax
    movl %eax, %cr0
    ret


#void asm_inthandler21(void)
asm_inthandler21:
    push %es
    push %ds
    pusha
    movl %esp, %eax
    push %eax
    movw %ss, %ax
    movw %ax, %ds
    movw %ax, %es
    call inthandler21
    pop %eax
    popa
    pop %ds
    pop %es
    iret
    #es, ds, ssを同じ値に揃えるのは、「C言語ではこれらが同じセグメントを指していると思いこむため」らしい

#void asm_inthandler2c(void)
asm_inthandler2c:
    push %es
    push %ds
    pusha
    mov %esp, %eax
    push %eax
    mov %ss, %ax
    mov %ax, %ds
    mov %ax, %es
    call inthandler2c
    pop %eax
    popa
    pop %ds
    pop %es
    iret

#void asm_inthandler27(void)
asm_inthandler27:
    push %es
    push %ds
    pusha
    mov %esp, %eax
    push %eax
    mov %ss, %ax
    mov %ax, %ds
    mov %ax, %es
    call inthandler27
    pop %eax
    popa
    pop %ds
    pop %es
    iret


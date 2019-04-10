.arch i486

.text

.global io_hlt, io_cli, io_sti, io_stihlt
.global io_in8, io_in16, io_in32
.global io_out8, io_out16, io_out32
.global io_load_eflags, io_store_eflags
.global load_gdtr, load_idtr
.global load_tr
.global farjmp
.global load_cr0, store_cr0
.global asm_inthandler21, asm_inthandler2c, asm_inthandler27, asm_inthandler20
.global memtest_sub

.global asm_hrb_api
.global farcall

.extern inthandler21, inthandler2c, inthandler27, inthandler20
.extern hrb_api

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

#void load_tr(int tr)
load_tr:
    ltr 4(%esp)
    ret

#void farjmp(int eip, int cs)
farjmp:
    ljmp *4(%esp)
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
    pushw %es
    pushw %ds
    pusha
    movl %esp, %eax
    pushl %eax
    movw %ss, %ax
    movw %ax, %ds
    movw %ax, %es
    call inthandler21
    popl %eax
    popa
    popw %ds
    popw %es
    iret
    #es, ds, ssを同じ値に揃えるのは、「C言語ではこれらが同じセグメントを指していると思いこむため」らしい

#void asm_inthandler2c(void)
asm_inthandler2c:
    pushw %es
    pushw %ds
    pusha
    mov %esp, %eax
    push %eax
    movw %ss, %ax
    movw %ax, %ds
    movw %ax, %es
    call inthandler2c
    popl %eax
    popa
    popw %ds
    popw %es
    iret

#void asm_inthandler27(void)
asm_inthandler27:
    pushw %es
    pushw %ds
    pusha
    mov %esp, %eax
    push %eax
    movw %ss, %ax
    movw %ax, %ds
    movw %ax, %es
    call inthandler27
    popl %eax
    popa
    popw %ds
    popw %es
    iret

#void asm_inthandler20(void)
asm_inthandler20:
    pushw %es
    pushw %ds
    pusha
    mov %esp, %eax
    push %eax
    movw %ss, %ax
    movw %ax, %ds
    movw %ax, %es
    call inthandler20
    popl %eax
    popa
    popw %ds
    popw %es
    iret

#unsigned int memtest_sub(unsigned int start, unsigned int end)
memtest_sub:
    push %edi #EDI, ESI, EBXも使う
    push %esi
    push %ebx
    movl $0xaa55aa55, %esi # pat0 = 0xaa55aa55
    movl $0x55aa55aa, %edi # pat1 = 0x55aa55aa
    movl 12+4(%esp), %eax  # i = start
mts_loop:
    movl %eax, %ebx
    addl $0xffc, %ebx # p = i + 0xffc  //下位4byteをチェックする
    movl (%ebx), %edx # old = *p
    movl %esi, (%ebx) # *p = pat0
    xorl $0xffffffff, (%ebx) # *p ^= 0xffffffff
    cmpl (%ebx), %edi # if(*p == pat1) goto mts_fin
    jne mts_fin
    xorl  $0xffffffff, (%ebx) # *p ^= 0xfffffff
    cmpl (%ebx), %esi # if(*p == pat1) goto mts_fin
    jne mts_fin
    movl %edx, (%ebx)
    addl $0x1000, %eax # i += 0x1000 (4KB進める)
    cmpl 12+8(%esp), %eax # if (i <= end) goto mts_loop
    jbe mts_loop
    pop %ebx
    pop %esi
    pop %edi
    ret
mts_fin:
    movl %edx, (%ebx) #*p = old
    pop %ebx
    pop %esi
    pop %edi
    ret

farcall:
    lcall *4(%esp)
    ret

asm_hrb_api:
    sti
    pusha #保存のため
    pusha #hrb_apiにわたすため
    call hrb_api
    add $32, %esp
    popa
    iret
    

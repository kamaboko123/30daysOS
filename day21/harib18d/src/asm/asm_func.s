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
.global start_app

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
    push %es
    push %ds
    pusha
    movw %ss, %ax
    cmpw $1 * 8, %ax
    jne from_app21
    
    #osが動いてるときに割り込まれたので今までどおり
    movl %esp, %eax
    push %ss #割り込まれたときのssを保存(これいる？)
    push %eax #割り込まれたときのespを保存
    movw %ss, %ax
    movw %ax, %ds
    movw %ax, %es
    call inthandler21
    add $8, %esp
    popa
    pop %ds
    pop %es
    iret
from_app21:
    movl $1 * 8, %eax
    movw %ax, %ds #DSをOS用に
    movl (0xfe4), %ecx #OS用ESP
    addl $-8, %ecx
    movw %ss, 4(%ecx) #割り込まれたときのssを保存
    movl %esp, (%ecx) #割り込まれたときのespを保存
    movw %ax, %ss
    movw %ax, %es
    movl %ecx, %esp
    call inthandler21
    pop %ecx
    pop %eax
    movw %ax, %ss
    movl %ecx, %esp
    popa
    pop %ds
    pop %es
    iret

#void asm_inthandler2c(void)
asm_inthandler2c:
    push %es
    push %ds
    pusha
    movw %ss, %ax
    cmpw $1 * 8, %ax
    #jne from_app2c
    
    #osが動いてるときに割り込まれたので今までどおり
    movl %esp, %eax
    push %ss #割り込まれたときのssを保存(これいる？)
    push %eax #割り込まれたときのespを保存
    movw %ss, %ax
    movw %ax, %ds
    movw %ax, %es
    call inthandler2c
    add $8, %esp
    popa
    pop %ds
    pop %es
    iret

from_app2c:
    movl $1 * 8, %eax
    movw %ax, %ds #DSをOS用に
    movl (0xfe4), %ecx #OS用ESP
    addl $-8, %ecx
    movw %ss, 4(%ecx) #割り込まれたときのssを保存
    movl %esp, (%ecx) #割り込まれたときのespを保存
    movw %ax, %ss
    movw %ax, %es
    movl %ecx, %esp
    call inthandler2c
    pop %ecx
    pop %eax
    movw %ax, %ss
    movl %ecx, %esp
    popa
    pop %ds
    pop %es
    iret


#void asm_inthandler27(void)
asm_inthandler27:
    push %es
    push %ds
    pusha
    movw %ss, %ax
    cmpw $1 * 8, %ax
    jne from_app27
    
    #osが動いてるときに割り込まれたので今までどおり
    movl %esp, %eax
    push %ss #割り込まれたときのssを保存(これいる？)
    push %eax #割り込まれたときのespを保存
    movw %ss, %ax
    movw %ax, %ds
    movw %ax, %es
    call inthandler27
    add $8, %esp
    popa
    pop %ds
    pop %es
    iret

from_app27:
    movl $1 * 8, %eax
    movw %ax, %ds #DSをOS用に
    movl (0xfe4), %ecx #OS用ESP
    addl $-8, %ecx
    movw %ss, 4(%ecx) #割り込まれたときのssを保存
    movl %esp, (%ecx) #割り込まれたときのespを保存
    movw %ax, %ss
    movw %ax, %es
    movl %ecx, %esp
    call inthandler27
    pop %ecx
    pop %eax
    movw %ax, %ss
    movl %ecx, %esp
    popa
    pop %ds
    pop %es
    iret

#void asm_inthandler20(void)
asm_inthandler20:
    push %es
    push %ds
    pusha
    movw %ss, %ax
    cmpw $1 * 8, %ax
    jne from_app20
    
    #osが動いてるときに割り込まれたので今までどおり
    movl %esp, %eax
    push %ss #割り込まれたときのssを保存
    push %eax #割り込まれたときのespを保存
    movw %ss, %ax
    movw %ax, %ds
    movw %ax, %es
    call inthandler20
    add $8, %esp
    popa
    pop %ds
    pop %es
    iret

from_app20:
    movl $1 * 8, %eax
    movw %ax, %ds #DSをOS用に
    movl (0xfe4), %ecx #OS用ESP
    addl $-8, %ecx
    movw %ss, 4(%ecx) #割り込まれたときのssを保存
    movl %esp, (%ecx) #割り込まれたときのespを保存
    movw %ax, %ss
    movw %ax, %es
    movl %ecx, %esp
    call inthandler20
    pop %ecx
    pop %eax
    movw %ax, %ss
    movl %ecx, %esp
    popa
    pop %ds
    pop %es
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

#void farcall(int eip, int cs);
farcall:
    lcall *4(%esp)
    ret

#void hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax)
asm_hrb_api:
    #cliされている
    
    push %ds
    push %es
    pusha #保存
    movl $1 * 8, %eax
    movw %ax, %ds #dsだけOS用に向ける
    movl (0xfe4), %ecx #OS用のESP (DSがOS用なので、start_appで記録しておいたやつを読み込む)
    add $-40, %ecx
    
    #dsはOS用に向いている
    #statr_appで保存したespを考えると、
    
    movl %esp, 32(%ecx) #アプリ用のESPを保存
    movw %ss, 36(%ecx) #アプリ用の用のSSを保存
    
    #pushaした値をシステムのスタックにコピーする
    #ssはアプリ、dsはOSを向いている
    movl (%esp), %edx
    movl 4(%esp), %ebx
    movl %edx, (%ecx) #コピー
    movl %ebx, 4(%ecx) #コピー
    
    movl 8(%esp), %edx
    movl 12(%esp), %ebx
    movl %edx, 8(%ecx) #コピー
    movl %ebx, 12(%ecx) #コピー
    
    movl 16(%esp), %edx
    movl 20(%esp), %ebx
    movl %edx, 16(%ecx) #コピー
    movl %ebx, 20(%ecx) #コピー
    
    movl 24(%esp), %edx
    movl 28(%esp), %ebx
    movl %edx, 24(%ecx) #コピー
    movl %ebx, 28(%ecx) #コピー
    
    #残りのセグメントレジスタもOS用に向ける
    movw %ax, %es
    movw %ax, %ss
    movl %ecx, %esp
    
    sti
    
    call hrb_api
    
    #espとssを戻すために読み込み
    movl 32(%esp), %ecx
    movl 36(%esp), %eax
    cli
    #戻す
    movw %ax, %ss
    movl %ecx, %esp
    popa
    pop %es
    pop %ds
    iret #自動でstiもやってくれる
    

#start_app(int eip, int cs, int esp, int ds)
start_app:
    pusha #レジスタ保存
    
    # pushaでレジスタ8個が積まれる
    # 8 * 4 = 32 byte分デクリメントされている
    # 一番左の引数(eip)は 32 + 4 = 36(%esp)にある
    movl 36(%esp), %eax #アプリのEIP
    movl 40(%esp), %ecx #アプリのCS
    movl 44(%esp), %edx #アプリのESP
    movl 48(%esp), %ebx #アプリのDS/SS
    
    movl %esp, (0xfe4) #OSのESP(戻ってきたときに使うために保存)
    cli #切り替え中に割り込みが起きないようにする
    
    movw %bx, %es
    movw %bx, %ss
    movw %bx, %ds
    movw %bx, %fs
    movw %bx, %gs
    movl %edx, %esp
    sti
    push %ecx #farcallに使う(cs)
    push %eax #farcallに使う(eip)
    lcall *(%esp)
    
    #OS用に戻す
    movl $1 * 8, %eax
    cli
    movw %ax, %es
    movw %ax, %ss
    movw %ax, %ds
    movw %ax, %fs
    movw %ax, %gs
    movl (0xfe4), %esp
    sti
    
    popa
    ret

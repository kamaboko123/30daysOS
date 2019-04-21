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
.global asm_inthandler0c, asm_inthandler0d, asm_inthandler21, asm_inthandler2c, asm_inthandler27, asm_inthandler20
.global memtest_sub

.global asm_hrb_api
.global farcall
.global start_app
.global asm_end_app

.extern inthandler0c, inthandler0d, inthandler21, inthandler2c, inthandler27, inthandler20
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

#void asm_inthandler0c(void)
asm_inthandler0c:
    sti
    push %es
    push %ds
    pusha
    movl %esp, %eax
    push %eax #espを記録しておく
    movw %ss, %ax #ds, esを揃える
    movw %ax, %ds
    movw %ax, %es
    call inthandler0c
    cmpl $0, %eax
    jne asm_end_app
    pop %eax
    popa
    pop %ds
    pop %es
    add $4, %esp #INT 0cではこれが必要
    iret

#void asm_inthandler0d(void)
asm_inthandler0d:
    sti
    push %es
    push %ds
    pusha
    movl %esp, %eax
    push %eax #espを記録しておく
    movw %ss, %ax #ds, esを揃える
    movw %ax, %ds
    movw %ax, %es
    call inthandler0d
    cmpl $0, %eax
    jne asm_end_app
    pop %eax
    popa
    pop %ds
    pop %es
    add $4, %esp #INT 0dではこれが必要
    iret

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

#void asm_inthandler2c(void)
asm_inthandler2c:
    push %es
    push %ds
    pusha
    mov %esp, %eax
    push %eax
    movw %ss, %ax
    movw %ax, %ds
    movw %ax, %es
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
    movw %ss, %ax
    movw %ax, %ds
    movw %ax, %es
    call inthandler27
    pop %eax
    popa
    pop %ds
    pop %es
    iret


#void asm_inthandler20(void)
asm_inthandler20:
    push %es
    push %ds
    pusha
    mov %esp, %eax
    push %eax
    movw %ss, %ax
    movw %ax, %ds
    movw %ax, %es
    call inthandler20
    pop %eax
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
    sti
    push %ds
    push %es
    pusha #保存
    pusha #hrb_apiに渡す用
    mov %ss, %ax #dsとesも揃える（必要？）
    mov %ax, %ds
    mov %ax, %es
    call hrb_api
    cmpl $0, %eax #戻り値チェック
    jne asm_end_app #0じゃなかったら終了する
    addl $32, %esp #pusha積んだ分を戻す
    popa #保存しておいたものを戻す
    pop %es
    pop %ds
    iret

asm_end_app:
    #eaxはtss.esp0の番地
    movl (%eax), %esp
    movl $0, 4(%eax)
    popa
    ret #cmd_appに戻る

#void start_app(int eip, int cs, int esp, int ds, int *tss_esp0)
start_app:
    pusha #レジスタ保存
    
    # pushaでレジスタ8個が積まれる
    # 8 * 4 = 32 byte分デクリメントされている
    # 一番左の引数(eip)は 32 + 4 = 36(%esp)にある
    movl 36(%esp), %eax #アプリのEIP
    movl 40(%esp), %ecx #アプリのCS
    movl 44(%esp), %edx #アプリのESP
    movl 48(%esp), %ebx #アプリのDS/SS
    movl 52(%esp), %ebp #tss.esp0の番地
    movl %esp, (%ebp) #OSのESPを保存(OSのtss.esp0に保存している)
    movw %ss, 4(%ebp) #OS用のSSを保存(tss.ss0に保存している)
    
    movw %bx, %es
    movw %bx, %ds
    movw %bx, %fs
    movw %bx, %gs
    
    #retfでアプリに飛ぶためにスタック調整
    #x86ではOSがアプリにcall/jmpしてはいけないことになっているらしい
    #なので、アプリの番地をpushしておいてlretで飛ぶ(P438)
    orl $3, %ecx #CS アプリのセグメント番号にorする（こういうものらしい？RPLとかいうやつ？）
    orl $3, %ebx #DS/SS アプリのセグメント番号にorする
    
    #以下をpushするのは特権レベルが違うセグメント間ではこうする必要がある？
    #lretは以下のような動作をする命令らしいので、ecx(アプリのcsを記録してる), eax(アプリのeipを記録している)をpushしている
    #pop %eip
    #pop %cs
    #jmp %cs:(%eip)
    #また、特権レベルが違うセグメント間でのジャンプでは、espとssも必要になるらしい
    #参考 : http://bttb.s1.valueserver.jp/wordpress/blog/2018/02/26/makeos-22/
    
    push %ebx #アプリのss
    push %edx #アプリのesp
    push %ecx #アプリのcs
    push %eax #アプリのeip
    
    lret



.text
.code16

.set BOTPAK, 0x00280000 # load destination of bootpack
.set DSKCAC, 0x00100000 # place of disk cache
.set DSKCAC0, 0x00008000 # place of disk cache(real mode)

# BOOT_INFO
.set CYLS, 0x0ff0
.set LEDS, 0x0ff1
.set VMODE, 0x0ff2 #n bit color
.set SCRNX, 0x0ff4 #display resolution X
.set SCRNY, 0x0ff6 #display resolution Y
.set VRAM, 0x0ff8 # head address of video memory

    # set video mode
    movb $0x13, %al # vga graphics 320x200 32bit color
    movb $0x00, %ah
    int $0x10

    # save screen information
    movb $8, (VMODE)
    movw $320, (SCRNX)
    movw $200, (SCRNY)
    movl $0x000a0000, (VRAM)

    # get keyboard led status from BIOS
    movb $0x02, %ah
    int $0x16
    movb %al, (LEDS)


    #PICが割り込みを受け付けないようにする
    #よくわからないけどAT互換機の仕様では、PICの初期化をする場合CLI前にやる必要があるらしい

    movb $0xff, %al
    outb %al, $0x21
    nop #out命令を連続させるとうまくいかない機種がある？らしい
    outb %al, $0xa1

    cli #CPUレベルでの割り込み禁止

    # A20GATEを設定(メモリを1MBまでアクセスできるようにする)
    call waitkbdout
    movb $0xd1, %al
    outb %al, $0x64
    call waitkbdout
    movb $0xdf, %al #enable A20
    outb %al, $0x60
    call waitkbdout


#protect mode

.arch i486

    lgdt (GDTR0) #暫定GDT
    movl %cr0, %eax
    andl $0x7ffffff, %eax
    orl $0x00000001, %eax
    movl %eax, %cr0
    jmp pipelineflush
pipelineflush:
    movw $1*8, %ax
    movw %ax, %dx
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

#bootpack転送
movl $bootpack, %esi
movl $BOTPAK, %edi
movl $512*1024/4, %ecx
call memcpy


#ディスクデータを本来の位置へ転送

#ブートセクタ
movl $0x7c00, %esi #転送元
movl $DSKCAC, %edi #転送先
movl $512/4, %ecx
call memcpy

#残り全部
movl $DSKCAC0+512, %esi
movl $DSKCAC+512, %edi
movl $0, %ecx
movb (CYLS), %cl
imull $512*18*2/4, %ecx #シリンダ数からバイト数/4に変換
sub $512/4, %ecx
call memcpy

#bootpack起動
movl $BOTPAK, %ebx
movl 16(%ebx), %ecx
add $3, %ecx #ECX+=3
SHR $2, %ecx #ECX/=4
jz skip #転送するべきものがない
movl 20(%ebx), %esi #転送元
add %ebx, %esi
movl 12(%ebx), %edi #転送先
call memcpy


skip:
    mov 12(%ebx), %esp #スタック初期化
    ljmpl $2*8, $0x0000001b


waitkbdout:
    inb $0x64, %al
    andb $0x02, %al
    #inb $0x60, %al #いらない？？
    jnz waitkbdout
    ret

memcpy:
    movl (%esi), %eax
    add $4, %esi
    movl %eax, (%edi)
    add $4, %edi
    sub $1, %ecx
    jnz memcpy #引き算結果が0でなければmemcpyへ
    ret


.align 16
GDT0:
    .skip 8, 0x00
    .word 0xffff, 0x0000, 0x9200, 0x00cf
    .word 0xffff, 0x0000, 0x9a28, 0x0047
    .word 0x0000

GDTR0:
    .word 8*3-1
    .int GDT0

bootpack:

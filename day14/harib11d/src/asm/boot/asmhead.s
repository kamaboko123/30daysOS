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
    movw $0x101, %bx # vbe 640 x 480 x 8bit-color
    movw $0x4f02, %ax
    int $0x10

    # save screen information
    movb $8, (VMODE)
    movw $640, (SCRNX)
    movw $480, (SCRNY)
    movl $0xfd000000, (VRAM)
    #本では0xe0000000をVRAMにしているが、何故かそれだと真っ暗になって動かなかった

    # get keyboard led status from BIOS
    movb $0x02, %ah
    int $0x16
    movb %al, (LEDS)


    #PICが割り込みを受け付けないようにする
    #よくわからないけどAT互換機の仕様では、PICの初期化をする場合CLI前にやる必要があるらしい
    #
    #io_out8(PIC0_IMR, 0xff)
    #io_out8(PIC0_IMR, 0xff)
    #をやっているのと同じ（IMRに0xffを送ると信号が来てもマスクされて無視する）
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
    #ここからプロテクトモードに入るための処理
    movl %cr0, %eax
    andl $0x7fffffff, %eax
    orl $0x00000001, %eax
    movl %eax, %cr0 #ページングを使用しないプロテクトモード(CR0の最上位bitを0, 最下位bitを1にする)
    jmp pipelineflush
pipelineflush:
    movw $1*8, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    #csレジスタは後回し
    #それ以外は0x0008に揃える(GDT + 1)

#bootpack転送
movl $bootpack, %esi #source (このasmheadの後ろにbootpackのバイナリをくっつけるので$bootpackはこのファイルの末尾)
movl $BOTPAK, %edi #destination
movl $512*1024/4, %ecx #memcpyのサイズはdouble word(4byte=32bit)単位なので、コピーするバイト数を4で割る
call memcpy


#ディスクデータを本来の位置へ転送

#ブートセクタ
movl $0x7c00, %esi #転送元
movl $DSKCAC, %edi #転送先
movl $512/4, %ecx
call memcpy

#残り全部
movl $DSKCAC0+512, %esi #source
movl $DSKCAC+512, %edi #destination
movl $0, %ecx
movb (CYLS), %cl
imull $512*18*2/4, %ecx #シリンダ数からバイト数/4に変換
sub $512/4, %ecx #ブートセクタ分を引く
call memcpy

#bootpack起動
movl $BOTPAK, %ebx
movl 16(%ebx), %ecx #データセクションのサイズ
add $3, %ecx #ECX+=3
SHR $2, %ecx #ECX/=4
jz skip #転送するべきものがない
movl 20(%ebx), %esi #転送元
add %ebx, %esi
movl 12(%ebx), %edi #転送先
call memcpy


skip:
    mov 12(%ebx), %esp #スタック初期化
    
    #特殊なjmp命令
    #CSに2*8が代入される
    #2番目のセグメントの0x1b番地にjmpする
    #実際には0x28001b番地(GDTで設定した通り)、これはbootpackの0x1b(=27byte目)番地にあたる
    #jmp先のデータはE9(JMP)で、bootpackのエントリポイントにjmpする（らしい？）
    #完全に理解した（わかってない）
    ljmpl $2*8, $0x0000001b
    
#キーボードの処理が終わるのを待つ
waitkbdout:
    inb $0x64, %al
    andb $0x02, %al
    #inb $0x60, %al #いらない？？
    jnz waitkbdout
    ret

#データのコピー
#esiのアドレスからediのアドレスにコピー 4byteずつecx回コピー
memcpy:
    movl (%esi), %eax
    add $4, %esi
    movl %eax, (%edi)
    add $4, %edi
    sub $1, %ecx
    jnz memcpy #引き算結果が0でなければmemcpyへ
    ret

#16byteアラインメント(GDT0ラベルが8の倍数になってないとパフォーマンスが落ちるらしい、バイト境界とかそのへんの話？)
.align 16
GDT0: 
    #bootpacckを動かすための仮のGDT
    #以下で設定されるものと同じ
    #set_segmdesc(gdt+1, 0xffffffff, 0x00000000, AR_DATA32_RW) 読み書き可能
    #set_segmdesc(gdt+2, LIMIT_BOTPAK, ADR_BOTPACK, AR_CODE32_ER) 実行可能(bootpack用)
    .skip 8, 0x00
    .word 0xffff, 0x0000, 0x9200, 0x00cf
    .word 0xffff, 0x0000, 0x9a28, 0x0047
    .word 0x0000

GDTR0:
    .word 8*3-1
    .long GDT0

bootpack:

.arch i486

.text

.global HariMain
HariMain:
    movw $1005 * 8, %ax
    movw %ax, %ds
    cmpl $0x69726148, %ds:0x0004 #シグネチャ(Hari)の検証、NASMだとsrcオペランドで'Hari'みたいに指定できるみたいけど、Gasでやり方が分からなかったので、とりあえずHEXで書いた
    jne fin #アプリではないので何もしない
    
    movl %ds:0x0000, %ecx #データセグメントの大きさを読み取る
    movw $2005 * 8, %ax
    movw %ax, %ds
    
crackloop:
    addl $-1, %ecx
    movb $123, %ds:(%ecx)
    cmpl $0, %ecx
    jne crackloop
    
fin:
    mov $4, %edx
    int $0x40



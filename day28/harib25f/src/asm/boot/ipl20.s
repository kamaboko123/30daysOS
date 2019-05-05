.text
.code16

jmp entry
.byte   0x90
.ascii "HELLOIPL"
.word   512 # sector size(should be 512 byte)
.byte   1 # clustor size (should be 1 sector)
.word   1 # start sector of FAT(normally 1 sector)
.byte   2 # number of FAT (should be 2)
.word   224 # size of root directory (normally 224)
.word   2880 # size of drive(should be2880 sector)
.byte   0xf0 # media type(should be 0xf0)
.word   9 # length of FAT area(should be 9 sector)
.word   18 # number of sector per track (should be 18 sector)
.word   2 # number of head(should be 2)
.int    0 # partion(should be 0 if not use partion)
.int    2880 # size of dirve
.byte   0,0,0x29 # unknown
.int    0xffffffff # maybe serial number ofvolume
.ascii  "HELLO-OS   " # disk name(11byte)
.ascii  "FAT12   " # name of format(8byte)
.skip   18,0 # padding?

.set CYLS, 20
.set _CYLS, 0xff0

entry:
    #init registers
    movw $0, %ax
    movw %ax, %ss
    movw $0x7c00, %sp
    movw %ax, %ds
    
    #load disk
    movw $0x0820, %ax
    movw %ax, %es #buffer address(ES:BX)
    movb $0, %ch #Cylinder 0
    movb $0, %dh # head 0
    movb $2, %cl # sector 2
    
readloop:
    movw $0, %si #retry counter
    
retry:
    movb $0x02, %ah # ah=0x02 read
    movb $1, %al #  1 sector
    movw $0, %bx #buffer address(ES:BX)
    movb $0x00, %dl # drive A:
    int $0x13 #intrupt bios
    jnc next # jump if not error
    
    add $1, %si #count up
    cmp $5, %si
    jae error
    
    #reset drive(drive A)
    movb $0x00, %ah
    movb $0x00, %dl # drive A:
    int $0x13
    
    jmp retry
    

next:
    movw %es, %ax
    add $0x20, %ax  #512 / 16 = 0x20
                    #対象のアドレスは(ES x 16 + BX)で決まるので、ESを0x20ずらすと、512byte分(1セクタ)ずらしたのとおなじになる
    movw %ax, %es #because instruction is not exist for add immidiate to es
    add $1, %cl
    cmp $18, %cl #read to sector 18
    jbe readloop
    
    movb $1, %cl # reset sector
    add $1, %dh # next head
    cmp $2, %dh # read to head 2
    jb readloop
    
    movb $0, %dh # reset head
    add $1, %ch # cylinder
    cmp $CYLS, %ch # read to cylinder CYLS
    jb readloop
    
    
    movb $CYLS, (_CYLS)
    jmp 0xc200  # jump to os program
                # 0x8000(buffer address) + 0x4200 (first file place of FAT12)
                # ES=0x0820でロードしている、メモリアドレスは(ESx16 + BX)なので、0x0820*16 + 0x00 = 0x8200
                # セクタ2から先をメモリアドレス0x82000を先頭としてロード(0x8000-0x8200の512byteはIPL分としてスキップ)
                # ディスクイメージ上0x4200の位置にあるOSのプログラムは、0x8000 + 0x4200 = 0xC200にロードされる
    
fin:
    hlt
    jmp fin

error:
    movw $msg, %si

putloop:
    movb (%si), %al
    addw $1, %si
    cmpb $0, %al
    je fin
    movb $0x0e, %ah
    movw $15, %bx
    int $0x10
    jmp putloop


msg:
    .string "\nload error\n\n"


//end of boot sector(must be 0x55, 0xaa)
.org 0x01fe
.byte 0x55, 0xaa


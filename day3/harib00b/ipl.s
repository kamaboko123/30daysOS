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
    
    movw $0, %si #retry counter
    
retry:
    movb $0x02, %ah # ah=0x02 read
    movb $1, %al #  1 sector
    movw $0, %bx #buffer address(ES:BX)
    movb $0x00, %dl # drive A:
    int $0x13 #intrupt bios
    jnc fin # jump if not error
    
    addw $1, %si #count up
    cmp $5, %si
    jae error
    
    #reset drive(drive A)
    movb $0x00, %ah
    movb $0x00, %dl # drive A:
    int $0x13
    
    jmp retry
    

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


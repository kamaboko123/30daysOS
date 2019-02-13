#helloos2

.byte   0xeb, 0x4e, 0x90
.ascii  "HELLOIPL"  # name of boot sector
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

# main program
.byte   0xb8, 0x00, 0x00, 0x8e, 0xd0, 0xbc, 0x00, 0x7c
.byte   0x8e, 0xd8, 0x8e, 0xc0, 0xbe, 0x74, 0x7c, 0x8a
.byte   0x04, 0x83, 0xc6, 0x01, 0x3c, 0x00, 0x74, 0x09
.byte   0xb4, 0x0e, 0xbb, 0x0f, 0x00, 0xcd, 0x10, 0xeb
.byte   0xee, 0xf4, 0xeb, 0xfd

#message
.byte   0x0a, 0x0a # 2 return
.ascii   "hello, world"
.byte   0x0a # return
.byte   0
.org    0x1fe # fill (0x00) to 0x001fe

.byte   0x55, 0xaa


# non boot sector
.byte   0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
.skip    4600
.byte   0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
.skip    1469432

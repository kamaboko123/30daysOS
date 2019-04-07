.arch i486

movb $'h', %al
lcall $2*8, $0x4c1f
movb $'e', %al
lcall $2*8, $0x4c1f
movb $'l', %al
lcall $2*8, $0x4c1f
movb $'l', %al
lcall $2*8, $0x4c1f
movb $'o', %al
lcall $2*8, $0x4c1f

lret


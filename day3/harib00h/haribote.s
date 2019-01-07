.text
.code16

.set CYLS, 0x0ff0
.set LEDS, 0x0ff1
.set VMODE, 0x0ff2 #n bit color
.set SCRNX, 0x0ff4 #display resolution X
.set SCRNY, 0x0ff6 #display resolution Y
.set VRAM, 0x0ff8 # head address of video memory

movb $0x13, %al # vga graphics 320x200 32bit color
movb $0x00, %ah
int $0x10

# save screen information
movb $8, (VMODE)
movw $320, (SCRNX)
movw $200, (SCRNY)
movl $0x000a0000, (VRAM)

# get keyboard led status from BIOS
mov $0x02, %ah
int $0x16
mov %al, (LEDS)

fin:
    hlt
    jmp fin


#include "bootpack.h"

void HariMain(void){
    char *vram;
    char str[32] = {0};
    char mcursor[16 * 16];
    int mx;
    int my;
    
    struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
    
    //GDT, IDTを初期化、PICを初期化、割り込みの受付を開始
    init_gdtidt();
    init_pic();
    io_sti();
    
    init_palette();
    init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
    
    init_mouse_cursor8(mcursor, COL8_008484);
    mx = (binfo->scrnx - 16) / 2;
    my = (binfo->scrny - 28 - 16) / 2;
    putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
    
    sprintf(str, "(%d, %d)", mx, my);
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, str);
    
    sprintf(str, "scrnx = %d", binfo->scrnx);
    putfonts8_asc(binfo->vram, binfo->scrnx, 16, 64, COL8_FFFFFF, str);
   
    putfonts8_asc(binfo->vram, binfo->scrnx, 31, 31, COL8_000000, "Haribote OS.");
    putfonts8_asc(binfo->vram, binfo->scrnx, 30, 30, COL8_FFFFFF, "Haribote OS.");
    
    
    //PIC1とキーボードを許可(11111001)
    io_out8(PIC0_IMR, 0xf9); 
    //マウスを許可(11101111)
    io_out8(PIC1_IMR, 0xef); 
    
    for(;;){
        io_hlt();
    }
}


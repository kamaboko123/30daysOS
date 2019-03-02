#include "bootpack.h"

void HariMain(void){
    char *vram;
    char str[32] = {0};
    char mcursor[16 * 16];
    char keybuf[KEYBUF_SIZE];
    char mousebuf[MOUSEBUF_SIZE];
    int mx;
    int my;
    int i;
    int j;
    unsigned int memtotal;
    
    struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
    struct MOUSE_DEC mdec;
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    
    struct SHTCTL *shtctl;
    struct SHEET *sht_back;
    struct SHEET *sht_mouse;
    unsigned char *buf_back;
    unsigned char buf_mouse[256];
    
    fifo8_init(&keyfifo, sizeof(keybuf), keybuf);
    fifo8_init(&mousefifo, sizeof(mousebuf), mousebuf);
    
    //GDT, IDTを初期化
    init_gdtidt();
    //PICを初期化
    init_pic();
    //キーボード初期化、マウス有効化
    init_keyboard();
    enable_mouse(&mdec);
    
    //割り込みの受付完了を開始
    io_sti();
    //PIC1とキーボードを許可(11111001)
    io_out8(PIC0_IMR, 0xf9); 
    //マウスを許可(11101111)
    io_out8(PIC1_IMR, 0xef); 
    
    
    //メモリ容量
    memtotal = memtest(0x00400000, 0xbfffffff);
    memman_init(memman);
    memman_free(memman, 0x00001000, 0x0009e000); //0x00001000 - 0x0009efff
    memman_free(memman, 0x00400000, memtotal - 0x00400000);
    
    init_palette();
    
    shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
    sht_back = sheet_alloc(shtctl);
    sht_mouse = sheet_alloc(shtctl);
    
    buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
    sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); //透明色なし
    sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
    
    init_screen(buf_back, binfo->scrnx, binfo->scrny);
    sheet_slide(shtctl, sht_back, 0, 0);
    
    init_mouse_cursor8(buf_mouse, 99);
    mx = (binfo->scrnx - 16) / 2;
    my = (binfo->scrny - 28 - 16) / 2;
    sheet_slide(shtctl, sht_mouse, mx, my);
    
    sheet_updown(shtctl, sht_back, 0);
    sheet_updown(shtctl, sht_mouse, 1);
    
    _sprintf(str, "(%d, %d)", mx, my);
    putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, str);
    
    _sprintf(str, "memory %dMB    free : %dKB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
    putfonts8_asc(buf_back, binfo->scrnx, 0, 32, COL8_FFFFFF, str);
    
    
    sheet_refresh(shtctl, sht_back, 0, 0, binfo->scrnx, 48);
    
    for(;;){
        io_cli();
        if(fifo8_status(&keyfifo) + fifo8_status(&mousefifo)== 0){
            io_stihlt();
        }
        else{
            //キーボード
            if(fifo8_status(&keyfifo) != 0){
                i = fifo8_get(&keyfifo);
                
                io_sti();
                _sprintf(str, "%02X", i);
                
                boxfill8(buf_back, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
                putfonts8_asc(buf_back, binfo->scrnx, 0, 16, COL8_FFFFFF, str);
                sheet_refresh(shtctl, sht_back, 0, 16, 16, 32);
            }
            //マウス
            else if(fifo8_status(&mousefifo) != 0){
                i = fifo8_get(&mousefifo);
                io_sti();
                
                if(mouse_decode(&mdec, i) != 0){
                    //_sprintf(str, "[lcr %04d %04d]", mdec.x, mdec.y);
                    _sprintf(str, "[lcr %04d %04d]", mdec.x, mdec.y);
                    
                    //1bit目 Left
                    if((mdec.btn & 0x01) != 0) str[1] = 'L';
                    //2bit目 center
                    if((mdec.btn & 0x04) != 0) str[2] = 'C';
                    //3bit目 Right
                    if((mdec.btn & 0x02) != 0) str[3] = 'R';
                    
                    boxfill8(buf_back, binfo->scrnx, COL8_008484, 32, 16, 32 + 15 * 8 - 1, 31);
                    putfonts8_asc(buf_back, binfo->scrnx, 32, 16, COL8_FFFFFF, str);
                    sheet_refresh(shtctl, sht_back, 32, 16, 32 + 15 * 8, 32);
                    
                    //値の書き換え
                    mx += mdec.x;
                    my += mdec.y;
                    
                    //画面外に行かないようにする
                    if(mx < 0) mx = 0;
                    if(my < 0) my = 0;
                    if(mx > binfo->scrnx - 16) mx = binfo->scrnx - 1;
                    if(my > binfo->scrny - 16) my = binfo->scrny - 1;
                    
                    //座標情報
                    _sprintf(str, "(%3d, %3d)", mx, my);
                    boxfill8(buf_back, binfo->scrnx, COL8_008484, 0, 0, 79, 15);
                    putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, str);
                    sheet_refresh(shtctl, sht_back, 0, 0, 80, 16);
                    
                    //移動後の描画
                    sheet_slide(shtctl, sht_mouse, mx, my);
                    
                }
            }
        }
    }
}


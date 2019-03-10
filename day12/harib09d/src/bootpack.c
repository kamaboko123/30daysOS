#include "bootpack.h"

void HariMain(void){
    char *vram;
    char str[32] = {0};
    char mcursor[16 * 16];
    char keybuf[KEYBUF_SIZE];
    char mousebuf[MOUSEBUF_SIZE];
    char timerbuf[8];
    char timerbuf2[8];
    char timerbuf3[8];
    int mx;
    int my;
    int i;
    int j;
    unsigned int memtotal;
    
    unsigned int count = 0;
    
    struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
    struct MOUSE_DEC mdec;
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    
    struct SHTCTL *shtctl;
    struct SHEET *sht_back;
    struct SHEET *sht_mouse;
    struct SHEET *sht_win;
    unsigned char *buf_back;
    unsigned char buf_mouse[256];
    unsigned char *buf_win;
    
    struct FIFO8 timerfifo;
    struct FIFO8 timerfifo2;
    struct FIFO8 timerfifo3;
    
    struct TIMER *timer;
    struct TIMER *timer2;
    struct TIMER *timer3;
    
    fifo8_init(&keyfifo, sizeof(keybuf), keybuf);
    fifo8_init(&mousefifo, sizeof(mousebuf), mousebuf);
   
        //GDT, IDTを初期化
    init_gdtidt();
    //PICを初期化
    init_pic();
    //タイマの初期化
    init_pit();
    //キーボード初期化、マウス有効化
    init_keyboard();
    enable_mouse(&mdec);
    
    //割り込みの受付完了を開始
    io_sti();
    //PIC1とPITとキーボードを許可(11111000)
    io_out8(PIC0_IMR, 0xf8); 
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
    sht_win = sheet_alloc(shtctl);
    
    buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
    buf_win = (unsigned char *)memman_alloc_4k(memman, 160 * 52);
    sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); //透明色なし
    sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
    sheet_setbuf(sht_win, buf_win, 160, 52, -1);
    
    init_screen(buf_back, binfo->scrnx, binfo->scrny);
    sheet_slide(sht_back, 0, 0);
    
    init_mouse_cursor8(buf_mouse, 99);
    mx = (binfo->scrnx - 16) / 2;
    my = (binfo->scrny - 28 - 16) / 2;
    sheet_slide(sht_mouse, mx, my);
    
    make_window8(buf_win, 160, 68, "counter");
    
    sheet_slide(sht_win, 80, 72);
    
    sheet_updown(sht_back, 0);
    sheet_updown(sht_win, 1);
    sheet_updown(sht_mouse, 2);
    
    _sprintf(str, "(%d, %d)", mx, my);
    putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, str);
    
    _sprintf(str, "memory %dMB    free : %dKB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
    putfonts8_asc(buf_back, binfo->scrnx, 0, 32, COL8_FFFFFF, str);
    
    sheet_refresh(sht_back, 0, 0, binfo->scrnx, 48);
    
    fifo8_init(&timerfifo, sizeof(timerbuf), timerbuf);
    timer = timer_alloc();
    timer_init(timer, &timerfifo, 1);
    timer_settime(timer, 1000);
    
    fifo8_init(&timerfifo2, sizeof(timerbuf2), timerbuf2);
    timer2 = timer_alloc();
    timer_init(timer2, &timerfifo2, 1);
    timer_settime(timer2, 300);
    
    fifo8_init(&timerfifo3, sizeof(timerbuf3), timerbuf3);
    timer3 = timer_alloc();
    timer_init(timer3, &timerfifo3, 1);
    timer_settime(timer3, 50);
    
    for(;;){
        _sprintf(str, "%010d", timerctl.count);
        boxfill8(buf_win, 160, COL8_C6C6C6, 40, 28, 119, 43);
        putfonts8_asc(buf_win, 160, 40, 28, COL8_000000, str);
        sheet_refresh(sht_win, 40, 28, 120, 44);
        
        io_cli();
        if(fifo8_status(&keyfifo) + fifo8_status(&mousefifo) + fifo8_status(&timerfifo) + fifo8_status(&timerfifo2) + fifo8_status(&timerfifo3) == 0){
            //io_stihlt();
            io_sti();
        }
        else{
            //キーボード
            if(fifo8_status(&keyfifo) != 0){
                i = fifo8_get(&keyfifo);
                
                io_sti();
                _sprintf(str, "%02X", i);
                
                boxfill8(buf_back, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
                putfonts8_asc(buf_back, binfo->scrnx, 0, 16, COL8_FFFFFF, str);
                sheet_refresh(sht_back, 0, 16, 16, 32);
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
                    sheet_refresh(sht_back, 32, 16, 32 + 15 * 8, 32);
                    
                    //値の書き換え
                    mx += mdec.x;
                    my += mdec.y;
                    
                    //画面外に行かないようにする
                    if(mx < 0) mx = 0;
                    if(my < 0) my = 0;
                    if(mx > binfo->scrnx - 1) mx = binfo->scrnx - 1;
                    if(my > binfo->scrny - 1) my = binfo->scrny - 1;
                    
                    //座標情報
                    _sprintf(str, "(%3d, %3d)", mx, my);
                    boxfill8(buf_back, binfo->scrnx, COL8_008484, 0, 0, 79, 15);
                    putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, str);
                    sheet_refresh(sht_back, 0, 0, 80, 16);
                    
                    //移動後の描画
                    sheet_slide(sht_mouse, mx, my);
                }
            }
            //タイマ
            else if(fifo8_status(&timerfifo) != 0){
                i = fifo8_get(&timerfifo);
                io_sti();
                putfonts8_asc(buf_back, binfo->scrnx, 0, 64, COL8_FFFFFF, "10[sec]");
                sheet_refresh(sht_back, 0, 64, 56, 80);
            }
            else if(fifo8_status(&timerfifo2) != 0){
                i = fifo8_get(&timerfifo2);
                io_sti();
                putfonts8_asc(buf_back, binfo->scrnx, 0, 80, COL8_FFFFFF, "3[sec]");
                sheet_refresh(sht_back, 0, 80, 48, 96);
            }
            else if(fifo8_status(&timerfifo3) != 0){
                i = fifo8_get(&timerfifo3);
                io_sti();
                if(i != 0){
                    timer_init(timer3, &timerfifo3, 0);
                    boxfill8(buf_back, binfo->scrnx, COL8_FFFFFF, 8, 96, 15, 111);
                }
                else{
                    timer_init(timer3, &timerfifo3, 1);
                    boxfill8(buf_back, binfo->scrnx, COL8_008484, 8, 96, 15, 111);
                }
                timer_settime(timer3, 50);
                sheet_refresh(sht_back, 8, 96, 16, 112);
            }
        }
    }
}

void make_window8(unsigned char *buf, int xsize, int ysize, char *title){
    static char closebtn[14][16] = {
        "OOOOOOOOOOOOOOO@",
        "OQQQQQQQQQQQQQ$@",
        "OQQQQQQQQQQQQQ$@",
        "OQQQ@@QQQQ@@QQ$@",
        "OQQQQ@@QQ@@QQQ$@",
        "OQQQQQ@@@@QQQQ$@",
        "OQQQQQQ@@QQQQQ$@",
        "OQQQQQ@@@@QQQQ$@",
        "OQQQQ@@QQ@@QQQ$@",
        "OQQQ@@QQQQ@@QQ$@",
        "OQQQQQQQQQQQQQ$@",
        "OQQQQQQQQQQQQQ$@",
        "O$$$$$$$$$$$$$$@",
        "@@@@@@@@@@@@@@@@"
    };
    
    int x, y;
    char c;
    
    boxfill8(buf, xsize, COL8_C6C6C6, 0, 0, xsize - 1, 0);
    boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, xsize - 2, 1);
    boxfill8(buf, xsize, COL8_C6C6C6, 0, 0, 0, ysize - 1);
    boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, 1, ysize - 2);
    boxfill8(buf, xsize, COL8_848484, xsize - 2, 1, xsize - 2, ysize - 2);
    boxfill8(buf, xsize, COL8_000000, xsize - 1, 0, xsize - 1, ysize - 1);
    boxfill8(buf, xsize, COL8_C6C6C6, 2, 2, xsize - 3, ysize - 3);
    boxfill8(buf, xsize, COL8_000084, 3, 3, xsize - 4, 20);
    boxfill8(buf, xsize, COL8_848484, 1, ysize - 2, xsize - 2, ysize - 2);
    boxfill8(buf, xsize, COL8_000000, 0, ysize - 1, xsize - 1, ysize - 1);
    
    putfonts8_asc(buf, xsize, 24, 4, COL8_FFFFFF, title);
    
    for(y = 0; y < 14; y++){
        for(x = 0; x < 16; x++){
            c = closebtn[y][x];
            if(c == '@') c = COL8_000000;
            else if(c == '$') c = COL8_848484;
            else if(c == 'Q') c = COL8_C6C6C6;
            else c = COL8_FFFFFF;
            
            buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
        }
    }
    
}

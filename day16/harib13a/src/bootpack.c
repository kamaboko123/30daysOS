#include "bootpack.h"

void HariMain(void){
    char *vram;
    char str[32] = {0};
    char mcursor[16 * 16];
    int fifobuf[128];
    int mx;
    int my;
    int i;
    int j;
    unsigned int memtotal;
    
    int cursor_x = 8;
    int cursor_c = COL8_FFFFFF;
    
    struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
    struct MOUSE_DEC mdec;
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
    
    struct SHTCTL *shtctl;
    struct SHEET *sht_back;
    struct SHEET *sht_mouse;
    struct SHEET *sht_win;
    unsigned char *buf_back;
    unsigned char buf_mouse[256];
    unsigned char *buf_win;
    
    struct FIFO32 fifo;
    
    struct TIMER *timer;
    struct TIMER *timer2;
    struct TIMER *timer3;
    
    static char keytable[] = {
        0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S',
        'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
        'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
        '2', '3', '0', '.'
    };
    
    struct TASK *task_b;
    
    fifo32_init(&fifo, 128, fifobuf);
    
    //GDT, IDTを初期化
    init_gdtidt();
    //PICを初期化
    init_pic();
    //タイマの初期化
    init_pit();
    //キーボード初期化、マウス有効化
    init_keyboard(&fifo, 256);
    enable_mouse(&fifo, 512, &mdec);
    
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
    
    make_window8(buf_win, 160, 52, "window");
    make_textbox8(sht_win, 8, 28, 144, 16, COL8_FFFFFF);
    
    
    sheet_slide(sht_win, 80, 72);
    
    sheet_updown(sht_back, 0);
    sheet_updown(sht_win, 1);
    sheet_updown(sht_mouse, 2);
    
    _sprintf(str, "(%d, %d)", mx, my);
    putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, str);
    
    _sprintf(str, "memory %dMB    free : %dKB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
    putfonts8_asc(buf_back, binfo->scrnx, 0, 32, COL8_FFFFFF, str);
    
    sheet_refresh(sht_back, 0, 0, binfo->scrnx, 48);
    
    timer = timer_alloc();
    timer_init(timer, &fifo, 10);
    timer_settime(timer, 1000);
    
    timer2 = timer_alloc();
    timer_init(timer2, &fifo, 3);
    timer_settime(timer2, 300);
    
    timer3 = timer_alloc();
    timer_init(timer3, &fifo, 1);
    timer_settime(timer3, 50);
    
    task_init(memman);
    task_b = task_alloc();
    
    task_b->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 8;
    task_b->tss.eip = (int)&task_b_main;
    task_b->tss.es = 1 * 8;
    task_b->tss.cs = 2 * 8;
    task_b->tss.ss = 1 * 8;
    task_b->tss.ds = 1 * 8;
    task_b->tss.fs = 1 * 8;
    task_b->tss.gs = 1 * 8;
    *((int *) (task_b->tss.esp + 4)) = (int) sht_back;
    
    task_run(task_b);
    
    for(;;){
        io_cli();
        if(fifo32_status(&fifo) == 0){
            io_stihlt();
        }
        else{
            i = fifo32_get(&fifo);
            io_sti();
            
            //キーボード
            if(i >= 256 && i <= 511){
                _sprintf(str, "%02X", i - 256);
                putfonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, str, 2);
                
                if(i < 256 + 0x54){
                    if(keytable[i - 256] != 0 && cursor_x < 144){
                        str[0] = keytable[i - 256];
                        str[1] = 0;
                        putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, str, 1);
                        cursor_x += 8;
                    }
                }
                if(i == 256 + 0x0e && cursor_x > 8){
                    putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
                    cursor_x -= 8;
                }
                
                boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
                sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
            }
            
            //マウス
            else if(i >= 512 && i <= 767){
                if(mouse_decode(&mdec, i - 512) != 0){
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
                    
                    //左クリックした位置にウインドウを移動する
                    if((mdec.btn & 0x01) != 0){
                        sheet_slide(sht_win, mx - 80, my - 8);
                    }
                }
            }
            //タイマ
            else if(i == 10){
                putfonts8_asc_sht(sht_back, 0, 64, COL8_FFFFFF, COL8_008484, "10[sec]", 7);
            }
            else if(i == 3){
                putfonts8_asc_sht(sht_back, 0, 80, COL8_FFFFFF, COL8_008484, "3[sec]", 6);
            }
            if(i <= 1){
                if(i != 0){
                    timer_init(timer3, &fifo, 0);
                    cursor_c = COL8_000000;
                }
                else{
                    timer_init(timer3, &fifo, 1);
                    cursor_c = COL8_FFFFFF;
                }
                timer_settime(timer3, 50);
                boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
                sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
            }
        }
    }
}

void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l){
    boxfill8(sht->buf, sht->bxsize, b, x, y, x + l * 8 - 1, y + 15);
    putfonts8_asc(sht->buf, sht->bxsize, x, y, c, s);
    sheet_refresh(sht, x, y, x + l * 8, y + 16);
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

void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c){
    int x1 = x0 + sx;
    int y1 = y0 + sy;
    
    boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 2, y0 - 3, x1 + 1, y0 - 3);
    boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 3, y0 - 3, x0 - 3, y1 + 1);
    boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x0 - 3, y1 + 2, x1 + 1, y1 + 2);
    boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x1 + 2, y0 - 3, x1 + 2, y1 + 2);
    boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 1, y0 - 2, x1 + 0, y0 - 2);
    boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 2, y0 - 2, x0 - 2, y1 + 0);
    boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x0 - 2, y1 + 1, x1 + 0, y1 + 1);
    boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x1 + 1, y0 - 2, x1 + 1, y1 + 1);
    boxfill8(sht->buf, sht->bxsize, c, x0 - 1, y0 - 1, x1 + 0, y1 + 0);
}

void task_b_main(struct SHEET *sht_back){
    struct FIFO32 fifo;
    struct TIMER *timer_put;
    struct TIMER *timer_1s;
    int i;
    int fifobuf[128];
    int count;
    int count0 = 0;
    char s[256];
    
    fifo32_init(&fifo, 128, fifobuf);
    
    timer_put = timer_alloc();
    timer_init(timer_put, &fifo, 1);
    
    timer_1s = timer_alloc();
    timer_init(timer_1s, &fifo, 100);
    timer_settime(timer_1s, 100);
    
    for(;;){
        count++;
        io_cli();
        
        if(fifo32_status(&fifo) == 0){
            io_sti();
        }
        else{
            i = fifo32_get(&fifo);
            io_sti();
            if(i == 100){
                _sprintf(s, "%11d", count - count0);
                putfonts8_asc_sht(sht_back, 0, 128, COL8_FFFFFF, COL8_008484, s, 11);
                count0 = count;
                timer_settime(timer_1s, 100);
            }
        }
    }
}

#include "bootpack.h"

void HariMain(void){
    char *vram;
    char s[32] = {0};
    char mcursor[16 * 16];
    int fifobuf[128];
    int keycmd_buf[32];
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
    struct SHEET *sht_cons;
    unsigned char *buf_back;
    unsigned char buf_mouse[256];
    unsigned char *buf_win;
    unsigned char *buf_cons;
    
    struct FIFO32 fifo;
    struct FIFO32 keycmd;
    
    struct TIMER *timer;
    
    static char keytable0[] = {
        0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S',
        'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
        'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
        '2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0x5c, 0,  0,   0,   0,   0,   0,   0,   0,   0,   0x5c, 0,  0
    };
    static char keytable1[] = {
        0,   0,   '!', 0x22, '#', '$', '%', '&', 0x27, '(', ')', '~', '=', '~', 0,   0,
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '`', '{', 0,   0,   'A', 'S',
        'D', 'F', 'G', 'H', 'J', 'K', 'L', '+', '*', 0,   0,   '}', 'Z', 'X', 'C', 'V',
        'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
        '2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   '_', 0,   0,   0,   0,   0,   0,   0,   0,   0,   '|', 0,   0
    };
    
    struct TASK *task_a;
    struct TASK *task_cons;
    
    int key_to = 0; //どのタスクに入力するか
    int key_shift = 0; //shiftの入力状態
    int key_leds = (binfo->leds >> 4) & 7; //キーボードの状態
    /*
        bit 4 -> ScrollLock
        bit 5 -> NumlLock
        bit 6 -> CapslLock
        ここではこの3bitを取り出す
    */
    int keycmd_wait = -1;
    
    fifo32_init(&fifo, 128, fifobuf, 0);
    fifo32_init(&keycmd, 32, keycmd_buf, 0);
    
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
    
    task_a = task_init(memman);
    fifo.task = task_a;
    task_run(task_a, 1, 0);
    
    init_palette();
    shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
    
    //back
    sht_back = sheet_alloc(shtctl);
    buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
    sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); //透明色なし
    init_screen8(buf_back, binfo->scrnx, binfo->scrny);
    
    //mouse
    sht_mouse = sheet_alloc(shtctl);
    sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
    init_mouse_cursor8(buf_mouse, 99);
    mx = (binfo->scrnx - 16) / 2;
    my = (binfo->scrny - 28 - 16) / 2;
    
    //window, task_b
    sht_cons = sheet_alloc(shtctl);
    buf_cons = (unsigned char *)memman_alloc_4k(memman, 256 * 165);
    sheet_setbuf(sht_cons, buf_cons, 256, 165, -1);
    make_window8(buf_cons, 256, 165, "console", 0);
    make_textbox8(sht_cons, 8, 28, 240, 128, COL8_000000);
    
    task_cons = task_alloc();
    task_cons->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 8;
    task_cons->tss.eip = (int)&console_task;
    task_cons->tss.es = 1 * 8;
    task_cons->tss.cs = 2 * 8;
    task_cons->tss.ss = 1 * 8;
    task_cons->tss.ds = 1 * 8;
    task_cons->tss.fs = 1 * 8;
    task_cons->tss.gs = 1 * 8;
    *((int *) (task_cons->tss.esp + 4)) = (int) sht_cons;
    task_run(task_cons, 2, 2);
    
    //window
    sht_win = sheet_alloc(shtctl);
    buf_win = (unsigned char *)memman_alloc_4k(memman, 144 * 52);
    sheet_setbuf(sht_win, buf_win, 144, 52, -1);
    make_window8(buf_win, 144, 52, "window", 1);
    make_textbox8(sht_win, 8, 28, 128, 16, COL8_FFFFFF);
    
    timer = timer_alloc();
    timer_init(timer, &fifo, 1);
    timer_settime(timer, 50);
    
    //init_screen(buf_back, binfo->scrnx, binfo->scrny);
    
    sheet_slide(sht_back, 0, 0);
    sheet_slide(sht_cons, 34, 4);
    sheet_slide(sht_win, 64, 56);
    sheet_slide(sht_mouse, mx, my);
    
    sheet_updown(sht_back, 0);
    sheet_updown(sht_cons, 1);
    sheet_updown(sht_win, 2);
    sheet_updown(sht_mouse, 3);
    
    _sprintf(s, "(%d, %d)", mx, my);
    putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);
    
    _sprintf(s, "memory %dMB    free : %dKB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
    putfonts8_asc_sht(sht_back, 0, 32, COL8_FFFFFF, COL8_008484, s, 40);
    
    //最初に設定しておく
    fifo32_put(&keycmd, KEYCMD_LED);
    fifo32_put(&keycmd, key_leds);
    
    for(;;){
        if(fifo32_status(&keycmd) > 0 && keycmd_wait < 0){
            //キーボードコントローラにデータを送信
            keycmd_wait = fifo32_get(&keycmd);
            wait_KBC_sendready();
            io_out8(PORT_KEYDAT, keycmd_wait);
        }
        
        io_cli();
        if(fifo32_status(&fifo) == 0){
            task_sleep(task_a);
            io_sti();
        }
        else{
            i = fifo32_get(&fifo);
            io_sti();
            
            //キーボード
            if(i >= 256 && i <= 511){
                _sprintf(s, "%02X", i - 256);
                putfonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 2);
                
                //キーコードを文字コードに変換
                if(i < 0x80 + 256){
                    if(key_shift == 0){
                        s[0] = keytable0[i - 256];
                    }
                    else{
                        s[0] = keytable1[i - 256];
                    }
                }
                else{
                    s[0] = 0;
                }
                
                if(s[0] >= 'A' && s[0] <= 'Z'){
                    //capslock off && shift off
                    //capslock on && shift on
                    //のときは小文字
                    if(((key_leds & 4) == 0 && key_shift == 0) ||
                        ((key_leds & 4) != 0 && key_shift != 0)){
                        s[0] += 0x20;
                    }
                }
                
                //通常文字
                if(s[0] != 0){
                    if(key_to == 0){ //task_a
                        if(cursor_x < 128){
                            s[1] = 0;
                            putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s, 1);
                            cursor_x += 8;
                        }
                    }
                    else { //task_cons
                        fifo32_put(&task_cons->fifo, s[0] + 256);
                    }
                }
                
                if(i == 256 + 0x0e){ //backspace
                    if(key_to == 0){
                        if(cursor_x > 8){
                            putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
                            cursor_x -= 8;
                        }
                    }
                    else{
                        fifo32_put(&task_cons->fifo, 8 + 256);
                    }
                }
                
                if(i == 256 + 0x0f){ //TAB
                    if(key_to == 0){
                        key_to = 1;
                        make_wtitle8(buf_win, sht_win->bxsize, "task_a", 0);
                        make_wtitle8(buf_cons, sht_cons->bxsize, "console", 1);
                        cursor_c = -1;
                        boxfill8(sht_win->buf, sht_win->bxsize, COL8_FFFFFF, cursor_x, 28, cursor_x + 7, 43);
                        fifo32_put(&task_cons->fifo, 2); //console
                    }
                    else{
                        key_to = 0;
                        make_wtitle8(buf_win, sht_win->bxsize, "task_a", 1);
                        make_wtitle8(buf_cons, sht_cons->bxsize, "console", 0);
                        cursor_c = COL8_000000;
                        fifo32_put(&task_cons->fifo, 3); //console
                    }
                    
                    sheet_refresh(sht_win, 0, 0, sht_win->bxsize, 21);
                    sheet_refresh(sht_cons, 0, 0, sht_cons->bxsize, 21);
                }
                
                if(i == 256 + 0x2a){ //left shift enable
                    key_shift |= 1;
                }
                if(i == 256 + 0x36){ //right shift enable
                    key_shift |= 2;
                }
                if(i == 256 + 0xaa){ //left shift disable
                    key_shift &= ~1;
                }
                if(i == 256 + 0xb6){ //right shift disable
                    key_shift &= ~2;
                }
                
                if(i == 256 + 0x3a){ //capslock
                    key_leds ^= 4;
                    fifo32_put(&keycmd, KEYCMD_LED);
                    fifo32_put(&keycmd, key_leds);
                }
                if(i == 256 + 0x45){ //numlock
                    key_leds ^= 2;
                    fifo32_put(&keycmd, KEYCMD_LED);
                    fifo32_put(&keycmd, key_leds);
                }
                if(i == 256 + 0x46){ //scrolllock
                    key_leds ^= 1;
                    fifo32_put(&keycmd, KEYCMD_LED);
                    fifo32_put(&keycmd, key_leds);
                }
                if(i == 256 + 0xfa){ //キーボードがデータを正しく受け取った
                    keycmd_wait = -1;
                }
                if(i == 256 + 0xfe){ //キーボードがデータを正しく受け取れなかった
                    wait_KBC_sendready();
                    io_out8(PORT_KEYDAT, keycmd_wait);
                }
                
                if(cursor_c >= 0){
                    boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
                }
                sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
            }
            
            //マウス
            else if(i >= 512 && i <= 767){
                if(mouse_decode(&mdec, i - 512) != 0){
                    //_sprintf(s, "[lcr %04d %04d]", mdec.x, mdec.y);
                    _sprintf(s, "[lcr %04d %04d]", mdec.x, mdec.y);
                    
                    //1bit目 Left
                    if((mdec.btn & 0x01) != 0) s[1] = 'L';
                    //2bit目 center
                    if((mdec.btn & 0x04) != 0) s[2] = 'C';
                    //3bit目 Right
                    if((mdec.btn & 0x02) != 0) s[3] = 'R';
                    
                    boxfill8(buf_back, binfo->scrnx, COL8_008484, 32, 16, 32 + 15 * 8 - 1, 31);
                    putfonts8_asc(buf_back, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
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
                    _sprintf(s, "(%3d, %3d)", mx, my);
                    boxfill8(buf_back, binfo->scrnx, COL8_008484, 0, 0, 79, 15);
                    putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
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
            if(i <= 1){
                if(i != 0){
                    timer_init(timer, &fifo, 0);
                    if(cursor_c >= 0){
                        cursor_c = COL8_000000;
                    }
                }
                else{
                    timer_init(timer, &fifo, 1);
                    if(cursor_c >= 0){
                        cursor_c = COL8_FFFFFF;
                    }
                }
                timer_settime(timer, 50);
                if(cursor_c >= 0){
                    boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
                    sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
                }
            }
        }
    }
}

void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l){
    boxfill8(sht->buf, sht->bxsize, b, x, y, x + l * 8 - 1, y + 15);
    putfonts8_asc(sht->buf, sht->bxsize, x, y, c, s);
    sheet_refresh(sht, x, y, x + l * 8, y + 16);
}

void make_window8(unsigned char *buf, int xsize, int ysize, char *title, char act){
    boxfill8(buf, xsize, COL8_C6C6C6, 0, 0, xsize - 1, 0);
    boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, xsize - 2, 1);
    boxfill8(buf, xsize, COL8_C6C6C6, 0, 0, 0, ysize - 1);
    boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, 1, ysize - 2);
    boxfill8(buf, xsize, COL8_848484, xsize - 2, 1, xsize - 2, ysize - 2);
    boxfill8(buf, xsize, COL8_000000, xsize - 1, 0, xsize - 1, ysize - 1);
    boxfill8(buf, xsize, COL8_C6C6C6, 2, 2, xsize - 3, ysize - 3);
    boxfill8(buf, xsize, COL8_848484, 1, ysize - 2, xsize - 2, ysize - 2);
    boxfill8(buf, xsize, COL8_000000, 0, ysize - 1, xsize - 1, ysize - 1);
    make_wtitle8(buf, xsize, title, act);
}

void make_wtitle8(unsigned char *buf, int xsize, char *title, char act){
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
    char tc;
    char tbc;
    
    if(act != 0){
        tc = COL8_FFFFFF;
        tbc = COL8_000084;
    }
    else{
        tc = COL8_C6C6C6;
        tbc = COL8_848484;
    }
    boxfill8(buf, xsize, tbc, 3, 3, xsize - 4, 20);
    putfonts8_asc(buf, xsize, 24, 4, tc, title);
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

void console_task(struct SHEET *sheet){
    struct TIMER *timer;
    struct TASK *task = task_now();
    
    int i;
    int fifobuf[128];
    int cursor_x = 16;
    int cursor_c = -2;
    char s[16];
    
    fifo32_init(&task->fifo, 128, fifobuf, task);
    
    timer = timer_alloc();
    timer_init(timer, &task->fifo, 1);
    timer_settime(timer, 50);
    
    putfonts8_asc_sht(sheet, 8, 28, COL8_FFFFFF, COL8_000000, ">", 1);
    
    for(;;){
        io_cli();
        if(fifo32_status(&task->fifo) == 0){
            task_sleep(task);
            io_sti();
        }
        else{
            i = fifo32_get(&task->fifo);
            io_sti();
            
            if(i <= 1){
                if(i != 0){
                    timer_init(timer, &task->fifo, 0);
                    if(cursor_c >= 0){
                        cursor_c = COL8_FFFFFF;
                    }
                }
                else{
                    timer_init(timer, &task->fifo, 1);
                    if(cursor_c >= 0){
                        cursor_c = COL8_000000;
                    }
                }
                
                timer_settime(timer, 50);
            }
            
            if(i == 2){ //cursor on
                cursor_c = COL8_FFFFFF;
            }
            if(i == 3){ //cursor off
                boxfill8(sheet->buf, sheet->bxsize, COL8_000000, cursor_x, 28, cursor_x + 7, 43);
                cursor_c = -1;
            }
            
            //キーボード
            if(i >= 256 && i <= 511){
                if(i == 8 + 256){ //backspace
                        if(cursor_x > 16){
                            putfonts8_asc_sht(sheet, cursor_x, 28, COL8_FFFFFF, COL8_000000, " ", 1);
                            cursor_x -= 8;
                        }
                }
                
                else{
                    if(cursor_x < 128){
                        s[0] = i - 256;
                        s[1] = 0;
                        putfonts8_asc_sht(sheet, cursor_x, 28, COL8_FFFFFF, COL8_000000, s, 1);
                        cursor_x += 8;
                    }
                }
                
                boxfill8(sheet->buf, sheet->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
                sheet_refresh(sheet, cursor_x, 28, cursor_x + 8, 44);
            }
            
            if(cursor_c >= 0){
               boxfill8(sheet->buf, sheet->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
            }
            sheet_refresh(sheet, cursor_x, 28, cursor_x + 8, 44);
        }
    }
}


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
    task_cons->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 12;
    task_cons->tss.eip = (int)&console_task;
    task_cons->tss.es = 1 * 8;
    task_cons->tss.cs = 2 * 8;
    task_cons->tss.ss = 1 * 8;
    task_cons->tss.ds = 1 * 8;
    task_cons->tss.fs = 1 * 8;
    task_cons->tss.gs = 1 * 8;
    *((int *) (task_cons->tss.esp + 4)) = (int) sht_cons;
    *((int *) (task_cons->tss.esp + 8)) = (int) memtotal;
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
                
                if(i == 256 + 0x1c){ //enter
                    if(key_to != 0){
                        fifo32_put(&task_cons->fifo, 10 + 256);
                    }
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
                    //値の書き換え
                    mx += mdec.x;
                    my += mdec.y;
                    
                    //画面外に行かないようにする
                    if(mx < 0) mx = 0;
                    if(my < 0) my = 0;
                    if(mx > binfo->scrnx - 1) mx = binfo->scrnx - 1;
                    if(my > binfo->scrny - 1) my = binfo->scrny - 1;
                    
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


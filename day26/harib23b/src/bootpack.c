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
    int x, y;
    int mmx = -1;
    int mmy = -1;
    int mmx2;
    unsigned int memtotal;
    
    struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
    struct MOUSE_DEC mdec;
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
    
    struct SHTCTL *shtctl;
    struct SHEET *sht_back;
    struct SHEET *sht_mouse;
    struct SHEET *sht_cons[2];
    unsigned char *buf_back;
    unsigned char buf_mouse[256];
    unsigned char *buf_cons[2];
    
    struct FIFO32 fifo;
    struct FIFO32 keycmd;
    int *cons_fifo[2];
    
    struct CONSOLE *cons;
    
    struct SHEET *sht = 0;
    struct SHEET *key_win;
    
    static char keytable0[] = {
        0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0x08,   0,
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0x0a,   0,   'A', 'S',
        'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
        'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
        '2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0x5c, 0,  0,   0,   0,   0,   0,   0,   0,   0,   0x5c, 0,  0
    };
    static char keytable1[] = {
        0,   0,   '!', 0x22, '#', '$', '%', '&', 0x27, '(', ')', '~', '=', '~', 0x08,   0,
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '`', '{', 0x0a,   0,   'A', 'S',
        'D', 'F', 'G', 'H', 'J', 'K', 'L', '+', '*', 0,   0,   '}', 'Z', 'X', 'C', 'V',
        'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
        '2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   '_', 0,   0,   0,   0,   0,   0,   0,   0,   0,   '|', 0,   0
    };
    
    struct TASK *task;
    struct TASK *task_a;
    struct TASK *task_cons[2];
    
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
    *((int *)0xfe4) = (int)shtctl;
    
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
    
    //console
    for(i = 0; i < 2; i++){
        sht_cons[i] = sheet_alloc(shtctl);
        buf_cons[i] = (unsigned char *)memman_alloc_4k(memman, 256 * 165);
        sheet_setbuf(sht_cons[i], buf_cons[i], 256, 165, -1);
        make_window8(buf_cons[i], 256, 165, "console", 0);
        make_textbox8(sht_cons[i], 8, 28, 240, 128, COL8_000000);
        
        task_cons[i] = task_alloc();
        task_cons[i]->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 12;
        task_cons[i]->tss.eip = (int)&console_task;
        task_cons[i]->tss.es = 1 * 8;
        task_cons[i]->tss.cs = 2 * 8;
        task_cons[i]->tss.ss = 1 * 8;
        task_cons[i]->tss.ds = 1 * 8;
        task_cons[i]->tss.fs = 1 * 8;
        task_cons[i]->tss.gs = 1 * 8;
        *((int *) (task_cons[i]->tss.esp + 4)) = (int) sht_cons[i];
        *((int *) (task_cons[i]->tss.esp + 8)) = (int) memtotal;
        task_run(task_cons[i], 2, 2); //level=2, priority=2
        sht_cons[i]->task = task_cons[i];
        sht_cons[i]->flags |= 0x20; //カーソルあり
        
        cons_fifo[i] = (int *) memman_alloc_4k(memman, 128 * 4);
        fifo32_init(&task_cons[i]->fifo, 128, cons_fifo[i], task_cons[i]);
    }
    
    sheet_slide(sht_back, 0, 0);
    sheet_slide(sht_cons[1], 56, 6);
    sheet_slide(sht_cons[0], 8, 2);
    sheet_slide(sht_mouse, mx, my);
    
    sheet_updown(sht_back, 0);
    sheet_updown(sht_cons[1], 1);
    sheet_updown(sht_cons[0], 2);
    sheet_updown(sht_mouse, 3);
    
    //最初に設定しておく
    fifo32_put(&keycmd, KEYCMD_LED);
    fifo32_put(&keycmd, key_leds);
    
    //とりあえず初期値はコンソールにしておく
    key_win = sht_cons[0];
    keywin_on(key_win);
    
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
            
            if(key_win->flags == 0){ //入力ウィンドウが閉じられた(ウインドウがなくなっていた)
                //2番目のウィンドウにむける
                key_win = shtctl->sheets[shtctl->top - 1];
                keywin_on(key_win);
            }
            
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
                
                //通常文字、
                if(s[0] != 0){
                    fifo32_put(&key_win->task->fifo, s[0] + 256);
                }
                
                if(i == 256 + 0x0f){ //TAB
                    keywin_off(key_win);
                    j = key_win->height - 1;
                    if(j == 0){
                        j = shtctl->top - 1;
                    }
                    key_win = shtctl->sheets[j];
                    keywin_on(key_win);
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
                
                if(i == 256 + 0x3b && key_shift != 0){ //shift + F1
                    task = key_win->task;
                    if(task != 0 && task->tss.ss0 != 0){
                        cons_putstr0(task->cons, "\nBreak(key) :\n");
                        //レジスタ変更中にタスクが変わらないようにする
                        io_cli();
                        task->tss.eax = (int) &(task->tss.esp0);
                        task->tss.eip = (int) asm_end_app;
                        io_sti();
                    }
                }
                
                if(i == 256 + 0x57 && shtctl->top > 2){ //F11
                    sheet_updown(shtctl->sheets[1], shtctl->top - 1);
                }
                
                if(i == 256 + 0xfa){ //キーボードがデータを正しく受け取った
                    keycmd_wait = -1;
                }
                if(i == 256 + 0xfe){ //キーボードがデータを正しく受け取れなかった
                    wait_KBC_sendready();
                    io_out8(PORT_KEYDAT, keycmd_wait);
                }
                
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
                    
                    //左クリック
                    if((mdec.btn & 0x01) != 0){
                        //通常モード
                        if(mmx < 0){
                            //上の下敷きから順番にマウスが押している下敷きを探す
                            for(j = shtctl->top - 1; j > 0; j--){
                                sht = shtctl->sheets[j];
                                x = mx - sht->vx0;
                                y = my - sht->vy0;
                                
                                //マウスが乗っているウインドウの判定
                                if(x >= 0 && x < sht->bxsize && 0 <= y && y < sht->bysize){
                                    //透明でない
                                    if(sht->buf[y * sht->bxsize + x] != sht->col_inv){
                                        sheet_updown(sht, shtctl->top - 1);
                                        
                                        if(sht != key_win){
                                            keywin_off(key_win);
                                            key_win = sht;
                                            keywin_on(key_win);
                                        }
                                        
                                        //タイトルバーを掴んだ
                                        if(x >= 3 && x < sht->bxsize - 3 && y >= 3 && y < 21){
                                            //ウインドウ移動モードにする
                                            mmx = mx;
                                            mmy = my;
                                            //もとの位置を覚えておく
                                            mmx2 = sht->vx0;
                                        }
                                        
                                        //閉じるボランのクリック
                                        if(sht->bxsize - 21 <= x && x < sht->bxsize - 5 && 5 <= y && y < 19){
                                            //アプリが作ったウィンドウ
                                            if((sht->flags & 0x10) != 0){
                                                task = sht->task;
                                                cons_putstr0(task->cons, "\nBreak(mouse):\n");
                                                
                                                //強制終了
                                                io_cli(); //強制終了中にタスクスイッチさせない
                                                task->tss.eax = (int) &(task->tss.esp0);
                                                task->tss.eip = (int) asm_end_app;
                                                io_sti();
                                            }
                                        }
                                        break;
                                    }
                                }
                            }
                        }
                        
                        else{
                            //ウインドウ移動モード
                            
                            //マウスの移動量を計算して移動
                            x = mx - mmx;
                            y = my - mmy;
                            
                            //xは高速化のために4で丸める(切り捨てにならないように2を足す)
                            //もとの位置を覚えておいて移動量が4の倍数になるようにする
                            sheet_slide(sht, (mmx2 + x + 2) & ~3, sht->vy0 + y);
                           
                            //移動先の座標に更新
                            mmy = my;
                        }
                    }
                    else{
                        //左ボタンを押していない
                        mmx = -1; //通常モードにする
                    }
                }
            }
        }
    }
}

void keywin_off(struct SHEET *key_win){
    change_wtitle8(key_win, 0);
    
    if((key_win->flags & 0x20) != 0){
        fifo32_put(&key_win->task->fifo, 3); //カーソルをoffにするためにfifoにデータを送る
    }
}

void keywin_on(struct SHEET *key_win){
    change_wtitle8(key_win, 1);
    
    if((key_win->flags & 0x20) != 0){
        fifo32_put(&key_win->task->fifo, 2); //カーソルをoffにするためにfifoにデータを送る
    }
}


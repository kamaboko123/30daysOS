#include "bootpack.h"

void HariMain(void){
    char s[32] = {0};
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
    int new_mx = -1;
    int new_my = 0;
    int new_wx = 0x7fffffff;
    int new_wy = 0;
    unsigned int memtotal;
    
    struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
    struct MOUSE_DEC mdec;
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    
    struct SHTCTL *shtctl;
    struct SHEET *sht_back;
    struct SHEET *sht_mouse;
    unsigned char *buf_back;
    unsigned char buf_mouse[256];
    
    struct FIFO32 fifo;
    struct FIFO32 keycmd;
    
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
    *((int *)0xfec) = (int)&fifo;
    
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
    key_win = open_console(shtctl, memtotal);
    
    sheet_slide(sht_back, 0, 0);
    sheet_slide(key_win, 32, 4);
    sheet_slide(sht_mouse, mx, my);
    
    sheet_updown(sht_back, 0);
    sheet_updown(key_win, 1);
    sheet_updown(sht_mouse, 2);
    
    //最初に設定しておく
    fifo32_put(&keycmd, KEYCMD_LED);
    fifo32_put(&keycmd, key_leds);
    
    //とりあえず初期値はコンソールにしておく
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
            //保留している描画があれば実行する
            if(new_mx >= 0){
                io_sti();
                //移動中はとりあえずnew_mx, new_myに移動先の座標を覚えているので、ここで描画する
                sheet_slide(sht_mouse, new_mx, new_my);
                new_mx = -1;
            }
            //0x7fffffffを使うのは、ウインドウの座標はマイナス値を取ることがあるから
            else if(new_wx != 0x7fffffff){
                io_sti();
                sheet_slide(sht, new_wx, new_wy);
                new_wx = 0x7fffffff;
            }
            else{
                task_sleep(task_a);
                io_sti();
            }
        }
        else{
            i = fifo32_get(&fifo);
            io_sti();
            
            if(key_win != 0 && key_win->flags == 0){ //入力ウィンドウが閉じられた(ウインドウがなくなっていた)
                //マウスと背景しかない場合
                if(shtctl->top == 1){
                    key_win = 0;
                }
                //他のウィンドウがある場合
                else{
                    //2番目のウィンドウにむける
                    key_win = shtctl->sheets[shtctl->top - 1];
                    keywin_on(key_win);
                }
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
                
                //通常文字、backspace、enter
                if(s[0] != 0 && key_win != 0){
                    fifo32_put(&key_win->task->fifo, s[0] + 256);
                }
                
                if(i == 256 + 0x0f && key_win != 0){ //TAB
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
                
                if(i == 256 + 0x3b && key_shift != 0 && key_win != 0){ //shift + F1
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
                if(i == 256 + 0x3c && key_shift != 0){ //shift + F2
                    if(key_win != 0){
                        keywin_off(key_win);
                    }
                    key_win = open_console(shtctl, memtotal);
                    sheet_slide(key_win, 32, 4);
                    sheet_updown(key_win, shtctl->top);
                    keywin_on(key_win);
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
                    
                    new_mx = mx;
                    new_my = my;
                    
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
                                            
                                            new_wy = sht->vy0;
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
                                            else{
                                                task = sht->task;
                                                io_cli();
                                                fifo32_put(&task->fifo, 4);
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
                            //ここでnew_wx, new_wyにウインドウの移動位置を覚えておいて、FIFOが空のときにslideを実行する
                            new_wx = (mmx2 + x + 2) & ~3;
                            new_wy = new_wy + y;
                            
                            //移動先の座標に更新
                            mmy = my;
                        }
                    }
                    else{
                        //左ボタンを押していない
                        mmx = -1; //通常モードにする
                        
                        //ボタンを離したときは、FIFOが空でなくてもすぐに描画する
                        if(new_wx != 0x7fffffff){
                            sheet_slide(sht, new_wx, new_wy); //一度確定させる
                            new_wx = 0x7fffffff;
                        }
                    }
                }
            }
            
            else if(i >= 768 && i <= 1023){ //consoleの終了処理(consoleでexitすると送られてくる)
                close_console(shtctl->sheets0 + (i - 768));
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


struct SHEET *open_console(struct SHTCTL *shtctl, unsigned int memtotal){
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    struct SHEET *sht;
    unsigned char *buf;
    struct TASK *task;
    int *cons_fifo;
    
    sht = sheet_alloc(shtctl);
    buf = (unsigned char *)memman_alloc_4k(memman, 256 * 165);
    sheet_setbuf(sht, buf, 256, 165, -1);
    make_window8(buf, 256, 165, "console", 0);
    make_textbox8(sht, 8, 28, 240, 128, COL8_000000);
    
    task = task_alloc();
    task->cons_stack = memman_alloc_4k(memman, 64 * 1024);
    task->tss.esp = task->cons_stack + 64 * 1024 - 12;
    task->tss.eip = (int)&console_task;
    task->tss.es = 1 * 8;
    task->tss.cs = 2 * 8;
    task->tss.ss = 1 * 8;
    task->tss.ds = 1 * 8;
    task->tss.fs = 1 * 8;
    task->tss.gs = 1 * 8;
    *((int *) (task->tss.esp + 4)) = (int) sht;
    *((int *) (task->tss.esp + 8)) = (int) memtotal;
    task_run(task, 2, 2); //level=2, priority=2
    sht->task = task;
    sht->flags |= 0x20; //カーソルあり
    
    cons_fifo = (int *) memman_alloc_4k(memman, 128 * 4);
    fifo32_init(&task->fifo, 128, cons_fifo, task);
    
    return sht;
}

void close_constask(struct TASK *task){
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    task_sleep(task);
    memman_free_4k(memman, task->cons_stack, 64 * 1024);
    memman_free_4k(memman, (int)task->fifo.buf, 128 * 4);
    task->flags = 0; //task_free(task)の代わり
}

void close_console(struct SHEET *sht){
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    struct TASK *task = sht->task;
    memman_free_4k(memman, (int)sht->buf, 256 * 165);
    sheet_free(sht);
    close_constask(task);
}

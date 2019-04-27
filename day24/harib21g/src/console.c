#include "bootpack.h"

void console_task(struct SHEET *sheet, unsigned int memtotal){
    struct TIMER *timer;
    struct TASK *task = task_now();
    
    int i;
    int x, y;
    int fifobuf[128];
    struct CONSOLE cons;
    char cmdline[30];
    cons.sht = sheet;
    cons.cur_x = 8;
    cons.cur_y = 28;
    cons.cur_c = -1;
    
    *((int *) 0xfec) = (int) &cons;
    
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    
    int *fat = (int *) memman_alloc_4k(memman, 4 * 2880);
    file_readfat(fat, (unsigned char *) (ADR_DISKIMG + 0x000200));
    
    fifo32_init(&task->fifo, 128, fifobuf, task);
    
    cons.timer = timer_alloc();
    timer_init(cons.timer, &task->fifo, 1);
    timer_settime(cons.timer, 50);
    
    cons_putchar(&cons, '>', 1);
    
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
                    timer_init(cons.timer, &task->fifo, 0);
                    if(cons.cur_c >= 0){
                        cons.cur_c = COL8_FFFFFF;
                    }
                }
                else{
                    timer_init(cons.timer, &task->fifo, 1);
                    if(cons.cur_c >= 0){
                        cons.cur_c = COL8_000000;
                    }
                }
                
                timer_settime(cons.timer, 50);
            }
            
            if(i == 2){ //cursor on
                cons.cur_c = COL8_FFFFFF;
            }
            if(i == 3){ //cursor off
                boxfill8(sheet->buf, sheet->bxsize, COL8_000000, cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);
                cons.cur_c = -1;
            }
            
            //キーボード
            if(i >= 256 && i <= 511){
                if(i == 8 + 256){ //backspace
                    if(cons.cur_x > 16){
                        cons_putchar(&cons, ' ', 0);
                        cons.cur_x -= 8;
                    }
                }
                else if(i == 10 + 256){ //enter
                    //カーソルをスペースで消して改行
                    cons_putchar(&cons, ' ', 0);
                    cmdline[cons.cur_x / 8 - 2] = 0;
                    cons_newline(&cons);
                    cons_runcmd(cmdline, &cons, fat, memtotal);
                    
                    //プロンプト表示
                    cons_putchar(&cons, '>', 1);
                }
                else{
                    if(cons.cur_x < 240){
                        cmdline[cons.cur_x / 8 - 2] = i - 256;
                        cons_putchar(&cons, i - 256, 1);
                    }
                }
            }
            
            if(cons.cur_c >= 0){
               boxfill8(sheet->buf, sheet->bxsize, cons.cur_c, cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);
            }
            sheet_refresh(sheet, cons.cur_x, cons.cur_y, cons.cur_x + 8, cons.cur_y + 16);
        }
    }
}

void cons_putchar(struct CONSOLE *cons, int chr, char move){
    char s[2];
    s[0] = chr;
    s[1] = 0;
    
    //tab
    if(s[0] == 0x09){
        for(;;){
            putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, " ", 1);
            cons->cur_x += 8;
            if(cons->cur_x == 8 + 240) cons_newline(cons);
            if(((cons->cur_x - 8) & 0x1f) == 0) break; //32で割り切れたら
        }
    }
    //LF
    else if(s[0] == 0x0a){
        cons_newline(cons);
    }
    else if(s[0] == 0x0d);
    else{
        putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 1);
        if(move != 0){ //moveが0のときはカーソルを進めない
            cons->cur_x += 8;
            if(cons->cur_x == 8 + 240) cons_newline(cons);
        }
    }
}

void cons_newline(struct CONSOLE *cons){
    int x, y;
    struct SHEET *sheet = cons->sht;
    
    if(cons->cur_y < 28 + 112){
        cons->cur_y += 16;
    }
    else{ //scroll
        for(y = 28; y < 28 + 112; y++){
            for(x = 8; x < 8 + 240; x++){
                sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (y + 16) * sheet->bxsize];
            }
        }
        for(y = 28 + 112; y < 28 + 128; y++){
            for(x = 8; x < 8 + 240; x++){
                sheet->buf[x + y * sheet->bxsize] = COL8_000000;
            }
        }
        sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
    }
    cons->cur_x = 8;
}


void cons_runcmd(char *cmdline, struct CONSOLE *cons, int *fat, unsigned int memtotal){
    if(_strcmp(cmdline, "mem") == 0){
        cmd_mem(cons, memtotal);
    }
    else if(_strcmp(cmdline, "cls") == 0){
        cmd_cls(cons);
    }
    else if(_strcmp(cmdline, "dir") == 0){
        cmd_dir(cons);
    }
    else if(_strncmp(cmdline, "type ", 5) == 0){
        cmd_type(cons, fat, cmdline);
    }
    else if(cmdline[0] != 0){
        if(cmd_app(cons, fat, cmdline) == 0){
            cons_putstr0(cons, "Bad Command.\n\n");
        }
    }
}

void cmd_mem(struct CONSOLE *cons, unsigned int memtotal){
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    char s[60];
    
    _sprintf(s, "total  %dMB\nfree   %dKB\n\n", memtotal / (1024 * 1024),  memman_total(memman) / 1024);
    cons_putstr0(cons, s);
}

void cmd_cls(struct CONSOLE *cons){
    int x,y;
    struct SHEET *sheet = cons->sht;
    
    for(y = 28; y < 28 + 128; y++){
        for(x = 8; x < 8 + 240; x++){
            sheet->buf[x + y * sheet->bxsize] = COL8_000000;
        }
    }
    sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
    cons->cur_y = 28;
}

void cmd_dir(struct CONSOLE *cons){
    struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);
    int i, j;
    char s[30];
    
   for(i = 0; i < 224; i++){
        if(finfo[i].name[0] == 0x00) break;
        if(finfo[i].name[0] != 0xa5){
            if((finfo[i].type & 0x18) == 0){
                _sprintf(s, "filename.ext    %7d", finfo[i].size);
                for(j = 0; j < 8; j++){
                    s[j] = finfo[i].name[j];
                }
                s[9] = finfo[i].ext[0];
                s[10] = finfo[i].ext[1];
                s[11] = finfo[i].ext[2];
                
                cons_putstr0(cons, s);
                cons_newline(cons);
            }
        }
    }
    cons_newline(cons);
}

void cmd_type(struct CONSOLE *cons, int *fat, char *cmdline){
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    struct FILEINFO *finfo = file_search(cmdline + 5, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
    
    char *p;
    int i;
    
    //ファイルが見つかった
    if(finfo != 0){
        p = (char *) memman_alloc_4k(memman, finfo->size);
        file_loadfile(finfo->clustno, finfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
        cons_putstr1(cons, p, finfo->size);
        memman_free_4k(memman, (int)p, finfo->size);
    }
    else{
        cons_putstr0(cons, "File not found.\n");
    }
    cons_newline(cons);
}

int cmd_app(struct CONSOLE *cons, int *fat, char *cmdline){
    int i;
    char *p;
    char *q;
    char name[18];
    
    char s[128];
    
    int segsiz;
    int esp;
    int datsiz;
    int dathrb;
    
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    struct FILEINFO *finfo;
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
    struct TASK *task = task_now();
    
    struct SHTCTL *shtctl;
    struct SHEET *sht;
    
    for(i = 0; i < 13; i++){
        if(cmdline[i] <= ' ') break;
        name[i] = cmdline[i];
    }
    
    name[i] = 0;
    
    //ファイルを探す
    finfo = file_search(name, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
    if(finfo == 0 && name[i - 1] != '.'){
        //拡張子付きでも探す
        name[i] = '.';
        name[i + 1] = 'H';
        name[i + 2] = 'R';
        name[i + 3] = 'B';
        name[i + 4] = 0;
        finfo = file_search(name, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
    }
    
    if(finfo != 0){
        p = (char *) memman_alloc_4k(memman, finfo->size);
        *((int *) 0xfe8) = (int) p;
        file_loadfile(finfo->clustno, finfo->size, p, fat, (char *)(ADR_DISKIMG + 0x003e00));
        
        //シグネチャのチェックと、mainを呼び出すように書き換え
        if(finfo->size >= 36 && _strncmp(p + 4, "Hari", 4) == 0 && *p == 0x00){
            
            //ヘッダから読み取る
            segsiz = *((int *) (p + 0x0000)); //データセグメントの大きさ
            esp = *((int *) (p + 0x000c)); //初期esp, データ転送先の初期位置
            datsiz = *((int *) (p + 0x0010)); //データセクションからデータセグメントにコピーする大きさ
            dathrb = *((int *) (p + 0x0014)); //hrbファイル内のデータセクションの位置
            
            //_sprintf(s, "[debug] segsiz=0x%X, esp=0x%X\n[debug] datsiz=0x%x, dathrb=0x%x\n", segsiz, esp, datsiz, dathrb);
            //cons_putstr0(cons, s);
            
            //データセグメントのサイズに基づいてメモリ確保
            q = (char *) memman_alloc_4k(memman, segsiz);
            
            //データセグメントを覚えておく(システムコールされたときにアプリケーションのデータのアクセスするのに必要)
            *((int *) 0xfe8) = (int)q;
            
            //0x60を足すのは、アプリのセグメントであるとあつかうため
            //コードセグメント
            set_segmdesc(gdt + 1003, finfo->size - 1, (int) p, AR_CODE32_ER + 0x60);
            //データセグメント
            set_segmdesc(gdt + 1004,  segsiz - 1, (int) q, AR_DATA32_RW + 0x60);
            
            //データセクションからデータセグメントにコピー
            for(i = 0; i < datsiz; i++){
                q[esp + i] = p[dathrb + i];
            }
            
            //権限による制御を使う場合は、TSSにOS用のセグメントと、ESPを登録する必要がある(P438)
            //0x1bから始めるのは、その位置(実際にはヘッダ内)に、mainへのjmp命令が埋め込まれてるから
            start_app(0x1b, 1003 * 8, esp, 1004 * 8, &(task->tss.esp0));
            shtctl = (struct SHTCTL *) *((int *) 0xfe4);
            
            for(i = 0; i < MAX_SHEETS; i++){
                sht = &(shtctl->sheets0[i]);
                //アプリが開きっぱなしのsheetは閉じる
                if((sht->flags & 0x11) == 0x11 && sht->task == task){
                    sheet_free(sht);
                }
            }
            
            memman_free_4k(memman, (int)q, segsiz);
        }
        else{
            cons_putstr0(cons, ".hrb file format error. \n");
        }
        
        memman_free_4k(memman, (int)p, finfo->size);
        cons_newline(cons);
        
        return 1;
    }
    
    return 0;
}

void cons_putstr0(struct CONSOLE *cons, char *s){
    for(; *s != 0; s++){
        cons_putchar(cons, *s, 1);
    }
}

void cons_putstr1(struct CONSOLE *cons, char *s, int l){
    int i;
    for(i = 0; i < l; i++){
        cons_putchar(cons, s[i], 1);
    }
}

int *hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax){
    int ds_base = *((int *) 0xfe8);
    struct TASK *task = task_now();
    struct CONSOLE *cons = (struct CONSOLE *) *((int *)0xfec);
    struct SHTCTL *shtctl = (struct SHTCTL *) *((int *)0xfe4);
    struct SHEET *sht;
    
    int i;
    
    int *reg = &eax + 1; //eaxの次の番地
    //asm_hrb_apiでこの関数はcallされ、call前にpsuhaを2回やっている
    //ここでは引数のeaxの次の番地(=1回目のpushadのedi)のアドレスを参照させる
    //1回目のpushはレジスタを保存するためのpushなので、eaxにpopされるであろうreg[7]に値をセットするとasm_hrb_apiの戻り値になる（無理やり）
    
    char s[32];
    
    if(edx == 1){
        cons_putchar(cons, eax & 0x000000ff, 1);
    }
    else if(edx == 2){
        cons_putstr0(cons, (char *)ebx + ds_base);
    }
    else if(edx == 3){
        cons_putstr1(cons, (char *)ebx + ds_base, ecx);
    }
    else if(edx == 4){
        //api_end
        return &(task->tss.esp0);
    }
    else if(edx == 5){
        /* windowを作る
         * ebx buf
         * esi xsiz
         * edi ysiz
         * eax col_in
         * ecx title
         * */
        
        sht = sheet_alloc(shtctl);
        sht->task = task;
        sht->flags |= 0x10;
        sheet_setbuf(sht, (char *) ebx + ds_base, esi, edi, eax);
        make_window8((char *) ebx + ds_base, esi, edi, (char *)ecx + ds_base, 0);
        sheet_slide(sht, 100, 50);
        sheet_updown(sht, 3);
        reg[7] = (int) sht;
    }
    else if(edx == 6){
        /*  windowに文字列を書く
         * ebx win
         * esi x
         * edi y
         * eax col
         * ecx len
         * ebp str
         * */
        
        sht = (struct SHEET *) (ebx & 0xfffffffe);
        putfonts8_asc(sht->buf, sht->bxsize, esi, edi, eax, (char *)ebp + ds_base);
        
        if((ebx & 1) == 0){
            sheet_refresh(sht, esi, edi, esi + ecx * 8, edi + 16);
        }
        
    }
    else if(edx == 7){
        /* windowに四角形を描く
         * ebx win
         * eax x0
         * ecx y0
         * esi x1
         * edi y1
         * ebp col
         * */
        sht = (struct SHEET *) (ebx & 0xfffffffe);
        boxfill8(sht->buf, sht->bxsize, ebp, eax, ecx, esi, edi);
        if((ebx & 1) == 0){
            sheet_refresh(sht, eax, ecx, esi + 1, edi + 1);
        }
    }
    else if(edx == 8){
        /* memman初期化
         * ebx : memman
         * eax : malloc開始アドレス（管理アドレスの最初）
         * ecx : 管理させる領域のバイト数
         * */
        memman_init((struct MEMMAN *) (ebx + ds_base));
        ecx &= 0xfffffff0; //16byte単位
        memman_free((struct MEMMAN *) (ebx + ds_base), eax, ecx);
    }
    else if(edx == 9){
        /* メモリ確保
         * ebx : memman
         * ecx : 要求バイト数
         * eax : 確保した領域のアドレス（戻り値）
         * */
        ecx = (ecx + 0x0f) & 0xfffffff0; //16byte単位に切り上げ
        reg[7] = memman_alloc((struct MEMMAN *)(ebx + ds_base), ecx);
    }
    else if(edx == 10){
        /* メモリ解放
         * ebx : memman
         * ebx : 開放する領域のアドレス
         * ecx : 開放したいバイト数
         * */
        ecx = (ecx + 0x0f) & 0xfffffff0; //16byte単位に切り上げ
        memman_free((struct MEMMAN *) (ebx + ds_base), eax, ecx);
    }
    else if(edx == 11){
        /* ウインドウに点を打つ
         * ebx : win
         * esi : x
         * edi : y
         * eax : color
         * */
        sht = (struct SHEET *) (ebx & 0xfffffffe);
        sht->buf[sht->bxsize * edi + esi] = eax;
        if((ebx & 1) == 0){
            sheet_refresh(sht, esi, edi, esi + 1, edi + 1);
        }
    }
    else if(edx == 12){
        /* refresh
         * ebx : win
         * eax : x0
         * ecx : y0
         * esi : x1
         * edi : y1
         * */
        
        sht = (struct SHEET *) ebx;
        sheet_refresh(sht, eax, ecx, esi, edi);
    }
    else if(edx == 13){
        /* ウインドウに線を描く
         * ebx : win
         * eax : x0
         * exx : y0
         * esi : x1
         * edi : y1
         * ebp : color
         * */
        
        sht = (struct SHEET *) (ebx & 0xfffffffe);
        hrb_api_linewin(sht, eax, ecx, esi, edi, ebp);
        if((ebx & 1) == 0){
            sheet_refresh(sht, esi, edi, esi + 1, edi + 1);
        }
    }
    else if(edx == 14){
        /* ウインドウを閉じる
         * ebx : win
         * */
        
        sheet_free((struct SHEET *) ebx);
    }
    else if(edx == 15){
        /**
         * キー入力を受け付ける
         * eax == 0 : キー入力がなければ-1を返す
         * eax == 1 : キー入力があるまでスリープ
         * */
        
        for(;;){
            io_cli();
            if(fifo32_status(&task->fifo) == 0){
                if(eax != 0) {
                    task_sleep(task); //待つ
                }
                else{
                    io_sti();
                    reg[7] = -1;
                    return 0;
                }
            }
            
            i = fifo32_get(&task->fifo);
            io_sti();
            
            if(i <= 1){ //カーソル用タイマ
                //アプリ実行中はカーソルが出ないので、いつも次は表示用の1にしておく
                timer_init(cons->timer, &task->fifo, 1);
                timer_settime(cons->timer, 50);
            }
            if(i == 2){ //カーソルon
                cons->cur_c = COL8_FFFFFF;
            }
            if(i == 3){
                cons->cur_c = -1;
            }
            if(i >= 256){
                reg[7] = i - 256;
                return 0;
            }
            
        }
    }
    else if(edx == 16){
        reg[7] = (int) timer_alloc();
    }
    else if(edx == 17){
        timer_init((struct TIMER *)ebx, &task->fifo, eax + 256);
    }
    else if(edx == 18){
        timer_settime((struct TIMER *)ebx, eax);
    }
    else if(edx == 19){
        timer_free((struct TIMER *)ebx);
    }
    
    return 0;
}

int *inthandler0d(int *esp){
    struct TASK *task = task_now();
    struct CONSOLE *cons = (struct CONSOLE *) *((int *) 0xfec);
    char s[30];
    cons_putstr0(cons, "\nINT 0D : \n General Protected Exception.\n");
    _sprintf(s, "EIP = %08X\n", esp[11]);
    cons_putstr0(cons, s);
    //異常終了
    return &(task->tss.esp0);
}

int *inthandler0c(int *esp){
    struct TASK *task = task_now();
    struct CONSOLE *cons = (struct CONSOLE *) *((int *) 0xfec);
    char s[30];
    cons_putstr0(cons, "\nINT 0C : \n Stack Exception.\n");
    _sprintf(s, "EIP = %08X\n", esp[11]);
    cons_putstr0(cons, s);
    //異常終了
    return &(task->tss.esp0);
}

void hrb_api_linewin(struct SHEET *sht, int x0, int y0, int x1, int y1, int col){
    int i;
    int x, y;
    int len;
    int dx, dy;
    
    dx = x1 - x0;
    dy = y1 - y0;
    
    //小数をまだ扱えないので整数を1024倍して扱う
    //シフト演算なので高速
    x = x0 << 10;
    y = y0 << 10;
    
    //変化量の絶対値を求めたいので、マイナスになった場合は整数にする
    if(dx < 0) dx = -dx;
    if(dy < 0) dy = -dy;
    
    //まずはlenを決める
    //変化量の大きい方をlenとする(lenは点を打つ回数、1024を足すと1ドット扱い)
    //変化量の大きい方は、変化量を1024か-1024(実際には線を引くときに1024で割るので1か-1)にする
    //変化量が小さい方はlenで割る(ここで1を足すのは、変化量が小さい方が丸められて、指定された位置まで足りなくなってしまうことを防ぐため)
    if(dx >= dy){
        len = dx + 1;
        //マイナス方向
        if(x0  > x1){
            dx -= 1024;
        }
        //プラス方向
        else{
            dx = 1024;
        }
        
        //短い方はlenで割る（書く回数で割って、長い方を1単位としたものに対する一回あたりの変化量とするなる）
        if(y0 <= y1){
            dy = ((y1 - y0 + 1) << 10) / len;
        }
        else{
            dy = ((y1 - y0 - 1) << 10) / len;
        }
    }
    else{
        len = dy + 1;
        if(y0 > y1){
            dy -= 1024;
        }
        else{
            dy = 1024;
        }
        if(x0 <= x1){
            dx = ((x1 - x0 + 1) << 10) / len;
        }
        else{
            dx = ((x1 - x0 - 1) << 10) / len;
        }
    }
    
    for(i = 0; i < len; i++){
        //それぞれの変化量ごとに描く
        sht->buf[(y >> 10) * sht->bxsize + (x >> 10)] = col;
        x += dx;
        y += dy;
    }
}


#include "bootpack.h"

void console_task(struct SHEET *sheet, unsigned int memtotal){
    struct TIMER *timer;
    struct TASK *task = task_now();
    
    int i;
    int x, y;
    int fifobuf[128];
    int cursor_x = 16;
    int cursor_c = -2;
    int cursor_y = 28;
    char s[30];
    char cmdline[30];
    char *p;
    
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);
    
    int *fat = (int *) memman_alloc_4k(memman, 4 * 2880);
    file_readfat(fat, (unsigned char *) (ADR_DISKIMG + 0x000200));
    
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
                boxfill8(sheet->buf, sheet->bxsize, COL8_000000, cursor_x, cursor_y, cursor_x + 7, cursor_y + 15);
                cursor_c = -1;
            }
            
            //キーボード
            if(i >= 256 && i <= 511){
                if(i == 8 + 256){ //backspace
                    if(cursor_x > 16){
                        putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
                        cursor_x -= 8;
                    }
                }
                else if(i == 10 + 256){ //enter
                    //カーソルをスペースで消す
                    putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
                    
                    cmdline[cursor_x / 8 - 2] = 0;
                    cursor_y = cons_newline(cursor_y, sheet);
                    
                    //コマンド実行
                    if(_strcmp(cmdline, "mem") == 0){
                        //mem
                        _sprintf(s, "total   %dMB", memtotal / (1024 * 1024));
                        putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
                        cursor_y = cons_newline(cursor_y, sheet);
                        _sprintf(s, "free    %dKB", memman_total(memman) / 1024);
                        putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
                        cursor_y = cons_newline(cursor_y, sheet);
                        cursor_y = cons_newline(cursor_y, sheet);
                    }
                    else if(_strcmp(cmdline, "cls") == 0){
                        for(y = 28; y < 28 + 128; y++){
                            for(x = 8; x < 8 + 240; x++){
                                sheet->buf[x + y * sheet->bxsize] = COL8_000000;
                            }
                        }
                        sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
                        cursor_y = 28;
                    }
                    else if(_strcmp(cmdline, "dir") == 0){
                        for(x = 0; x < 224; x++){
                            if(finfo[x].name[0] == 0x00) break;
                            
                            if(finfo[x].name[0] != 0xa5){
                                if((finfo[x].type & 0x18) == 0){
                                    _sprintf(s, "filename.ext    %7d", finfo[x].size);
                                    for(y = 0; y < 8; y++){
                                        s[y] = finfo[x].name[y];
                                    }
                                    s[9] = finfo[x].ext[0];
                                    s[10] = finfo[x].ext[1];
                                    s[11] = finfo[x].ext[2];
                                    
                                    putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
                                    cursor_y = cons_newline(cursor_y, sheet);
                                }
                            }
                        }
                        cursor_y = cons_newline(cursor_y, sheet);
                    }
                    else if(_strncmp(cmdline, "type ", 5) == 0){
                        //ファイル名を取得
                        for(y = 0; y < 11; y++){
                            s[y] = ' ';
                        }
                        y = 0;
                        for(x = 5; y < 11 && cmdline[x] != 0; x++){
                            if(cmdline[x] == '.' && y <= 8) y = 8;
                            else{
                                s[y] = cmdline[x];
                                if(s[y] >= 'a' && s[y] <= 'z') s[y] -= 0x20;
                                y++;
                            }
                        }
                        
                        //ファイルを探す
                        for(x = 0; x < 224; ){
                            if(finfo[x].name[0] == 0x00) break;
                            
                            if((finfo[x].type & 0x18) == 0){
                                for(y = 0; y < 11; y++){
                                    if(finfo[x].name[y] != s[y]){
                                        goto type_next_file;
                                    }
                                }
                                break; //ファイルが見つかった
                            }
type_next_file:
                            x++;
                        }
                        
                        //見つかった
                        if(x < 224 && finfo[x].name[0] != 0x00){
                            p = (char *)memman_alloc_4k(memman, finfo[x].size);
                            file_loadfile(finfo[x].clustno, finfo[x].size, p, fat, (char *)(ADR_DISKIMG + 0x003e00));
                            
                            cursor_x = 8;
                            
                            for(y = 0; y < finfo[x].size; y++){
                                s[0] = p[y];
                                s[1] = 0;
                                
                                //tab
                                if(s[0] == 0x09){
                                    for(;;){
                                        putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
                                        cursor_x += 8;
                                        if(cursor_x == 8 + 240){
                                            cursor_x = 8;
                                            cursor_y = cons_newline(cursor_y, sheet);
                                        }
                                        
                                        //32で割り切れるまで
                                        if((cursor_x - 8) & 0x1f) break;
                                    }
                                }
                                
                                //改行(LF)
                                else if(s[0] == 0x0a){
                                    cursor_x = 8;
                                    cursor_y = cons_newline(cursor_y, sheet);
                                }
                                
                                //CR
                                else if(s[0] == 0x0d){}
                                else{
                                        putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, s, 1);
                                        cursor_x += 8;
                                    //改行
                                    if(cursor_x == 8 + 240){
                                        cursor_x = 8;
                                        cursor_y = cons_newline(cursor_y, sheet);
                                    }
                                }
                            }
                            memman_free_4k(memman, (int)p, finfo[x].size);
                        }
                        else{
                            putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, "File not found.", 15);
                            cursor_y = cons_newline(cursor_y, sheet);
                        }
                        cursor_y = cons_newline(cursor_y, sheet);
                    }
                    else if(cmdline[0] != 0){
                        putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, "Bad command", 12);
                        cursor_y = cons_newline(cursor_y, sheet);
                        cursor_y = cons_newline(cursor_y, sheet);
                    }
                    
                    //プロンプト表示
                    putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, ">", 1);
                    cursor_x = 16;
                }
                else{
                    if(cursor_x < 128){
                        s[0] = i - 256;
                        s[1] = 0;
                        cmdline[cursor_x / 8 - 2] = i - 256;
                        putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, s, 1);
                        cursor_x += 8;
                    }
                }
            }
            
            if(cursor_c >= 0){
               boxfill8(sheet->buf, sheet->bxsize, cursor_c, cursor_x, cursor_y, cursor_x + 7, cursor_y + 15);
            }
            sheet_refresh(sheet, cursor_x, cursor_y, cursor_x + 8, cursor_y + 16);
        }
    }
}

int cons_newline(int cursor_y, struct SHEET *sheet){
    int x, y;
    
    if(cursor_y < 28 + 112){
        cursor_y += 16;
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
    
    return cursor_y;
}


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
    
    init_palette();
    init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
    
    init_mouse_cursor8(mcursor, COL8_008484);
    mx = (binfo->scrnx - 16) / 2;
    my = (binfo->scrny - 28 - 16) / 2;
    putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
    
    _sprintf(str, "(%d, %d)", mx, my);
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, str);
    
    //_sprintf(str, "scrnx = %d", binfo->scrnx);
    //putfonts8_asc(binfo->vram, binfo->scrnx, 16, 64, COL8_FFFFFF, str);
    /*
    putfonts8_asc(binfo->vram, binfo->scrnx, 33, 33, COL8_000000, "Haribote OS.");
    putfonts8_asc(binfo->vram, binfo->scrnx, 32, 32, COL8_FFFFFF, "Haribote OS.");
    */
    
    //メモリ容量
    memtotal = memtest(0x00400000, 0xbfffffff);
    memman_init(memman);
    memman_free(memman, 0x00001000, 0x0009e000); //0x00001000 - 0x0009efff
    memman_free(memman, 0x00400000, memtotal - 0x00400000);
    
    _sprintf(str, "memory %dMB    free : %dKB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 32, COL8_FFFFFF, str);
    
    //PIC1とキーボードを許可(11111001)
    io_out8(PIC0_IMR, 0xf9); 
    //マウスを許可(11101111)
    io_out8(PIC1_IMR, 0xef); 
    
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
                
                boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
                putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, str);
                
                _sprintf(str, "keybuf(r,w) = (%d : %d)", keyfifo.q, keyfifo.p);
                boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 92, binfo->scrnx, 107);
                putfonts8_asc(binfo->vram, binfo->scrnx, 0, 92, COL8_FFFFFF, str);
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
                    
                    boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 320, 31);
                    putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, str);
                    
                    //マウス移動
                    //消す
                    boxfill8(binfo->vram, binfo->scrnx, COL8_008484, mx, my, mx + 15, my + 15);
                    
                    //値の書き換え
                    mx += mdec.x;
                    my += mdec.y;
                    
                    //画面外に行かないようにする
                    if(mx < 0) mx = 0;
                    if(my < 0) my = 0;
                    if(mx > binfo->scrnx - 16) mx = binfo->scrnx - 16;
                    if(my > binfo->scrny - 16) my = binfo->scrny - 16;
                    
                    //座標情報
                    _sprintf(str, "(%3d, %3d)", mx, my);
                    boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 0, 79, 15);
                    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, str);
                    
                    //マウス描画
                    putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
                }
            }
        }
    }
}

#define EFLAGS_AC_BIT 0x00040000 //18(AC) bit
#define CR0_CACHE_DISABLE 0x60000000 //30(CD), 21(PG) bit

unsigned int memtest(unsigned int start, unsigned int end){
    char flg486 = 0;
    unsigned int eflg;
    unsigned int cr0;
    unsigned int i;
    
    //486以降ならキャッシュを無効にする必要がある
    
    //386か486かを確認する
    //EFLAGSを読み込んで、ac-bit(第18bit)を1にしてEFLAGSに戻す
    //386ではac-bitを1にしても自動的に0に戻ってしまうので、これで386か486かを判定できる
    eflg = io_load_eflags();
    eflg |= EFLAGS_AC_BIT;
    io_store_eflags(eflg);
    eflg = io_load_eflags();
    if((eflg & EFLAGS_AC_BIT) != 0) flg486 = 1;
    
    //ac-bitは0に戻しておく
    eflg &= ~EFLAGS_AC_BIT;
    io_store_eflags(eflg);
    
    //486以降ならキャッシュを無効にする
    //crにあるのフラグで設定できる
    if(flg486 != 0){
        cr0 = load_cr0();
        cr0 |= CR0_CACHE_DISABLE;
        store_cr0(cr0);
    }
    
    i = memtest_sub(start, end);
    
    //キャッシュを有効に戻す
    if(flg486 != 0){
        cr0 = load_cr0();
        cr0 &= ~CR0_CACHE_DISABLE;
        store_cr0(cr0);
    }
    
    return i;
}



void memman_init(struct MEMMAN *man){
    man->frees = 0;
    man->maxfrees = 0;
    man->lostsize = 0;
    man->losts = 0;
}

unsigned int memman_total(struct MEMMAN *man){
    unsigned int i;
    unsigned int t = 0;
    
    for(i = 0; i < man->frees; i++){
        t += man->free[i].size;
    }
    
    return t;
}

unsigned int memmam_alloc(struct MEMMAN *man, unsigned int size){
    unsigned int i;
    unsigned int a;
    
    for(i = 0; i < man->frees; i++){
        if(man->free[i].size >= size){
            //確保可能
            a = man->free[i].addr;
            man->free[i].addr += size;
            man->free[i].size -= size;
            
            //free[i]がなくなったので前へ詰める
            if(man->free[i].size == 0){
                man->frees--;
                for(; i < man->frees; i++){
                    man->free[i] = man->free[i + 1];
                }
            }
            
            return a;
        }
    }
    
    return 0;
}

int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size){
    int i;
    int j;
    
    //挿入場所を決める（freeがaddr順に並んでいたほうがまとめやすい）
    for(i = 0; i < man->frees; i++){
        if(man->free[i].addr > addr) break;
    }
    
    // free[i - 1].addr < addr < free[i]
    if(i > 0){
        //iが0より大きい＝前がある
        
        //まとめられる
        if(man->free[i - 1].addr + man->free[i - 1].size == addr){
            man->free[i - 1].size += size;
            
            //freesより小さい、後ろもある
            if(i < man->frees){
                
                //後ろもまとめられる
                if(addr + size == man->free[i].addr){
                    man->free[i-1].size += man->free[i].size;
                    
                    //後ろをまとめてfree[i]が空いたのでつめる
                    man->frees--;
                    for(; i < man->frees; i++){
                        man->free[i] = man->free[i + 1];
                    }
                }
            }
            
            return 0;
        }
    }
    
    //後ろがある
    if(i < man->frees){
        //まとめられる
        if(addr + size == man->free[i].addr){
            man->free[i].addr = addr;
            man->free[i].size += size;
            
            return 0;
        }
    }
    
    //以下まとめられない場合
    
    //リストに追加可能
    if(man->frees < MEMMAN_FREES){
        //リストに追加するために、後ろへつめる
        for(j = man->frees; j > i; j--){
            man->free[j] = man->free[j - 1];
        }
        //追加
        man->frees++;
        if(man->maxfrees < man->frees){
            man->maxfrees = man->frees; //最大値更新
        }
        
        man->free[i].addr = addr;
        man->free[i].size = size;
        return 0;
    }
    
    //リストに追加できない（エラー）
    man->losts++;
    man->lostsize += size;
    
    return -1;
}


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
    
    struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
    struct MOUSE_DEC mdec;
    
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
    //最大3GBまで
    i = memtest(0x004000000, 0xbfffffff) / (1024 * 1024);
    _sprintf(str, "memory %dMB", i);
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

unsigned int memtest_sub(unsigned int start, unsigned int end){
    unsigned int i;
    unsigned int *p;
    unsigned int old;
    
    unsigned int pat0 = 0xaa55aa55;
    unsigned int pat1 = 0x55aa55aa;
    
    for(i = start; i <= end; i += 4){
        p = (unsigned int *)i;
        
        //戻せるように今の値を覚えておく
        old = *p;
        
        //テスト用のデータを入れてbit反転
        *p = pat0;
        *p ^= 0xffffffff;
        
        //正常に反転できていなければ値をもどして抜ける
        if(*p != pat1){
            *p = old;
            break;
        }
        
        //再反転して同じように値をチェックする
        *p ^= 0xffffffff;
        
        if(*p != pat0){
            *p = old;
            break;
        }
    }
    
    return i;
}


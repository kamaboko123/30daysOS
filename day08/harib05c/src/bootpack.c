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
    
    putfonts8_asc(binfo->vram, binfo->scrnx, 33, 33, COL8_000000, "Haribote OS.");
    putfonts8_asc(binfo->vram, binfo->scrnx, 32, 32, COL8_FFFFFF, "Haribote OS.");
    
    
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
                }
                
                _sprintf(str, "mousebuf(r,w) = (%d : %d)", mousefifo.q, mousefifo.p);
                boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 109, binfo->scrnx, 125);
                putfonts8_asc(binfo->vram, binfo->scrnx, 0, 109, COL8_FFFFFF, str);
            }
        }
    }
}

void wait_KBC_sendready(void){
    //キーボードコントローラの準備ができるまで待つ
    //port 0x0064の2bit目が0になったら準備完了なので抜ける
    
    for(;;){
        if((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) break;
    }
}

void init_keyboard(void){
    //キーボードコントローラ初期化
    
    wait_KBC_sendready();
    //モード設定のためのコマンド(0x60)
    io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
    wait_KBC_sendready();
    //マウスを利用できるようにする(0x47)
    io_out8(PORT_KEYDAT, KBC_MODE);
}

void enable_mouse(struct MOUSE_DEC *mdec){
    //マウス有効化
    
    wait_KBC_sendready();
    //マウスを設定
    //キーボードコントローラに0xd4を送るとマウスに転送してくれる
    io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
    wait_KBC_sendready();
    //マウスを有効化
    //成功するとACK(0xfa)が返ってくるらしい
    io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
    
    //デコード用構造体の初期化
    mdec->phase = 0;
}

int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat){
    if(mdec->phase == 0){
        if(dat == 0xfa) mdec->phase++;
        
        return 0;
    }
    
    if(mdec->phase == 1){
        
        //正しいデータかチェック
        //0xc8 = 11001000
        //上位4bitはマウスの動きに合わせて0-3の範囲で変化する
        //下位4bitはクリックで8-Fの範囲で変化する
        //受け取るデータは常に、00XX1XXXXになるので、0x8cとの論理積をとって0x08(00001000)で無ければ不正
        if((dat & 0xc8) == 0x08){
            mdec->buf[0] = dat;
            mdec->phase = 2;
        }
        
        return 0;
    }
    
    if(mdec->phase == 2){
        mdec->buf[1] = dat;
        mdec->phase = 3;
        
        return 0;
    }
    
    if(mdec->phase == 3){
        mdec->buf[2] = dat;
        mdec->phase = 1;
        
        mdec->btn = mdec->buf[0] & 0x07;
        mdec->x = mdec->buf[1];
        mdec->y = mdec->buf[2];
        
        //x, yは基本的に2, 3byte目のデータをそのまま使う
        //1byte目のx, yそれぞれに対応するbitが1だと上位24bitを全部1になる
        if((mdec->buf[0] & 0x10) != 0){
            mdec->x |= 0xffffff00;
        }
        if((mdec->buf[0] & 0x20) != 0){
            mdec->y |= 0xffffff00;
        }
        
        //y方向の符号反転
        mdec->y = -mdec->y;
        
        return 1;
    }
    
    return -1;
}



#include "bootpack.h"

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

//マウス割り込み
void inthandler2c(int *esp){
    unsigned char data;
    
    //IRQ-12(スレーブの4番)受付完了をPIC1に通知
    io_out8(PIC1_OCW2, 0x64);
    //IRQ-02受付完了をPIC0に通知
    io_out8(PIC0_OCW2, 0x62);
    
    data = io_in8(PORT_KEYDAT);
    fifo8_put(&mousefifo, data);
}


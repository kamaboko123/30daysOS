#include "bootpack.h"

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

//キーボード割り込み
void inthandler21(int *esp){
    struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
    unsigned char data;
    unsigned char s[128];
    
    //IRQ-01に受付完了を通知
    io_out8(PIC0_OCW2, 0x61);
    data = io_in8(PORT_KEYDAT);
    
    fifo8_put(&keyfifo, data);
}


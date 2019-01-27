#include "bootpack.h"

struct FIFO8 keyfifo;
struct FIFO8 mousefifo;

void init_pic(){
    /*
     * IMR : interrupt mask register
     * このビットが1になっているIRQからの信号を無視する
     * 設定中や、使用しない場合は1を入れておく
     * 
     * ICW : init control word
     * ICW1, 4はここでの値以外は使われない（割り込み信号の電気的な特性に関連する）
     * ICW3はマスタ・スレーブの接続関係、マスタには何番のIRQにスレーブがつながっているかを8bitで指定
     * （最大8個接続できるけど、普通は1個でIRQ2で接続される）
     * スレーブはマスタの何番につながっているかを指定する
     * ICW2は割り込みの番号を設定する、ここでは0x20-0x2fで受け取る
     * ちなみに0x00-0x1fは他で使うので、ここでは使えない
     */
    
    io_out8(PIC0_IMR, 0xff); //すべての割り込みを受け付けない
    io_out8(PIC1_IMR, 0xff); //すべての割り込みを受け付けない
    
    io_out8(PIC0_ICW1, 0x11); //エッジトリガモード
    io_out8(PIC0_ICW2, 0x20); //IRQ0-7はINT20-27で受ける
    io_out8(PIC0_ICW3, 1 << 2); //PIC1はIRQ2にて接続
    io_out8(PIC0_ICW4, 0x01); //ノンバッファモード
    
    io_out8(PIC1_ICW1, 0x11); //エッジトリガモード
    io_out8(PIC1_ICW2, 0x28); //IRQ0-7はINT20-27で受ける
    io_out8(PIC1_ICW3, 2); //PIC1はIRQ2にて接続
    io_out8(PIC1_ICW4, 0x01); //ノンバッファモード
    
    io_out8(PIC0_IMR, 0xfb); //11111011 PIC1以外はすべて禁止
    io_out8(PIC1_IMR, 0xff); //11111111 すべての割り込みを受け付けない
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

void inthandler27(int *esp){
    io_out8(PIC0_OCW2, 0x67); /* IRQ-07受付完了をPICに通知(7-1参照) */
    return;
}

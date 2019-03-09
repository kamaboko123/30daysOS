#include "bootpack.h"

struct TIMERCTL timerctl;

void init_pit(void){
    io_out8(PIT_CTRL, 0x34);
    io_out8(PIT_CNT0, 0x9c);
    io_out8(PIT_CNT0, 0x2e);
    
    timerctl.count = 100;
}

void inthandler20(int *esp){
    //割り込み受付を通知
    io_out8(PIC0_OCW2, 0x60);
    timerctl.count++;
}


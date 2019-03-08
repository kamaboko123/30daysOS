#include "bootpack.h"

void init_pit(void){
    io_out8(PIT_CTL, 0x34);
    io_out8(PIT_CNT0, 0x9c);
    io_out8(PIT_CNT0, 0x2e);
}

void inthandler20(int *esp){
    io_out8(PIC0_OCW2, 0x60);
}


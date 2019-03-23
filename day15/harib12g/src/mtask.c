#include "bootpack.h"

struct TIMER *mt_timer;
int mt_tr;

void mt_init(void){
    mt_timer = timer_alloc();
    //fifoにデータを入れないのでinit不要
    timer_settime(mt_timer, 2);
    mt_tr = 3 * 8;
}

void mt_taskswitch(void){
    if(mt_tr == 3 * 8){
        mt_tr = 4 * 8;
    }
    else{
        mt_tr = 3 * 8;
    }
    timer_settime(mt_timer, 2);
    farjmp(0, mt_tr);
}

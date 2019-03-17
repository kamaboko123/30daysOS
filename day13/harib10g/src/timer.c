#include "bootpack.h"

struct TIMERCTL timerctl;

void init_pit(void){
    int i;
    
    io_out8(PIT_CTRL, 0x34);
    io_out8(PIT_CNT0, 0x9c);
    io_out8(PIT_CNT0, 0x2e);
    
    timerctl.count = 0;
    timerctl.next = 0xFFFFFFFF;
    timerctl.using = 0;
    for(i = 0; i < MAX_TIMER; i++){
        timerctl.timers0[i].flags = 0;
    }
}

struct TIMER *timer_alloc(void){
    int i;
    for(i = 0; i < MAX_TIMER; i++){
        if(timerctl.timers0[i].flags == 0){
            timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;
            return &timerctl.timers0[i];
        }
    }
    return 0;
}

void timer_free(struct TIMER *timer){
    timer->flags = 0;
}

void timer_init(struct TIMER *timer, struct FIFO32 *fifo, unsigned char data){
    timer->fifo = fifo;
    timer->data = data;
}

void timer_settime(struct TIMER *timer, unsigned int timeout){
    int e;
    int i, j;
    timer->timeout = timerctl.count + timeout;
    timer->flags = TIMER_FLAGS_USING;
    
    e = io_load_eflags();
    
    //挿入場所を探す(i)
    for(i = 0; i < timerctl.using; i++){
        if(timerctl.timers[i]->timeout >= timer->timeout) break;
    }
    
    //i番目以降をずらす
    for(j = timerctl.using; j > i; j--){
        timerctl.timers[j] = timerctl.timers[j - 1];
    }
    timerctl.using++;
    
    timerctl.timers[i] = timer;
    timerctl.next = timerctl.timers[i]->timeout;
    
    io_store_eflags(e);
}

void inthandler20(int *esp){
    int i, j;
    
    //割り込み受付を通知
    io_out8(PIC0_OCW2, 0x60);
    timerctl.count++;
    
    if(timerctl.next > timerctl.count) return;
    
    //タイムアウトしたタイマの数を数える(timersにはタイムアウトが早い順に並ぶ)
    for(i = 0; i < timerctl.using; i++){
        //timersのタイマはすべて動作中のタイマなので、flagsは見なくていい
        if(timerctl.timers[i]->timeout > timerctl.count) break;
        
        //timeout
        timerctl.timers[i]->flags = TIMER_FLAGS_ALLOC;
        fifo32_put(timerctl.timers[i]->fifo, timerctl.timers[i]->data);
    }
    
    //タイムアウトした分を引く
    timerctl.using -= i;
    //タイムアウトした個数分ずらす
    for(j = 0; j < timerctl.using; j++){
        timerctl.timers[j] = timerctl.timers[i + j];
    }
    
    //残りの動作中のタイマがあればnextを更新
    if(timerctl.using > 0){
        timerctl.next = timerctl.timers[0]->timeout;
    }
    else{
        timerctl.next = 0xffffffff;
    }
}


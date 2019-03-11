#include "bootpack.h"

struct TIMERCTL timerctl;

void init_pit(void){
    int i;
    
    io_out8(PIT_CTRL, 0x34);
    io_out8(PIT_CNT0, 0x9c);
    io_out8(PIT_CNT0, 0x2e);
    
    timerctl.count = 0;
    timerctl.next = 0xFFFFFFFF;
    for(i = 0; i < MAX_TIMER; i++){
        timerctl.timer[i].flags = 0;
    }
}

struct TIMER *timer_alloc(void){
    int i;
    for(i = 0; i < MAX_TIMER; i++){
        if(timerctl.timer[i].flags == 0){
            timerctl.timer[i].flags = TIMER_FLAGS_ALLOC;
            return &timerctl.timer[i];
        }
    }
    return 0;
}

void timer_free(struct TIMER *timer){
    timer->flags = 0;
}

void timer_init(struct TIMER *timer, struct FIFO8 *fifo, unsigned char data){
    timer->fifo = fifo;
    timer->data = data;
}

void timer_settime(struct TIMER *timer, unsigned int timeout){
    timer->timeout = timerctl.count + timeout;
    timer->flags = TIMER_FLAGS_USING;
    
    //すでにセットされているタイマーよりも短ければ更新
    if(timerctl.next > timer->timeout){
        timerctl.next = timer->timeout;
    }
}

void inthandler20(int *esp){
    int i;
    
    //割り込み受付を通知
    io_out8(PIC0_OCW2, 0x60);
    timerctl.count++;
    
    if(timerctl.next > timerctl.count) return;
    
    for(i = 0; i < MAX_TIMER; i++){
        if(timerctl.timer[i].flags == TIMER_FLAGS_USING){
            if(timerctl.timer[i].timeout <= timerctl.count){
                //timeout
                timerctl.timer[i].flags = TIMER_FLAGS_ALLOC;
                fifo8_put(timerctl.timer[i].fifo, timerctl.timer[i].data);
            }
            else{
                //not timeout
                if(timerctl.next > timerctl.timer[i].timeout){
                    timerctl.next = timerctl.timer[i].timeout;
                }
            }
        }
    }
    
    for(i = 0; i < MAX_TIMER; i++){
        if(timerctl.timer[i].flags == TIMER_FLAGS_USING){
            if(timerctl.timer[i].timeout <= timerctl.count){
                timerctl.timer[i].flags = TIMER_FLAGS_ALLOC;
                fifo8_put(timerctl.timer[i].fifo, timerctl.timer[i].data);
            }
        }
    }
}


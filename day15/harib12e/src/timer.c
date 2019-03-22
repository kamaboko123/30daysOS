#include "bootpack.h"

struct TIMERCTL timerctl;

void init_pit(void){
    int i;
    struct TIMER *t;
    
    io_out8(PIT_CTRL, 0x34);
    io_out8(PIT_CNT0, 0x9c);
    io_out8(PIT_CNT0, 0x2e);
    
    timerctl.count = 0;
    for(i = 0; i < MAX_TIMER; i++){
        timerctl.timers0[i].flags = 0;
    }
    
    //dummy
    t = timer_alloc();
    t->timeout = 0xffffffff;
    t->flags = TIMER_FLAGS_USING;
    t->next = 0;
    timerctl.t0 = t;
    timerctl.next = 0xffffff;
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

void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data){
    timer->fifo = fifo;
    timer->data = data;
}

void timer_settime(struct TIMER *timer, unsigned int timeout){
    int e;
    struct TIMER *t;
    struct TIMER *s;
    
    timer->timeout = timerctl.count + timeout;
    timer->flags = TIMER_FLAGS_USING;
    
    e = io_load_eflags();
    io_cli();
    
    //先頭
    t = timerctl.t0;
    if(timer->timeout <= t->timeout){
        timerctl.t0 = timer;
        timer->next = t;
        timerctl.next = timer->timeout;
        io_store_eflags(e);
        return;
    }
    
    for(;;){
        s = t;
        t = t->next;
        //最後まで行ったら抜ける
        if(t == 0) break;
        
        if(timer->timeout <= t->timeout){
            //sとtの間に入れる
            //sはtの一つ前なので、この間に入れればいい
            //中間への挿入なので、timerctl->nextなどは更新不要
            s->next = timer;
            timer->next = t;
            io_store_eflags(e);
            return;
        }
    }
}

void inthandler20(int *esp){
    int i, j;
    struct TIMER *timer;
    
    //割り込み受付を通知
    io_out8(PIC0_OCW2, 0x60);
    timerctl.count++;
    
    if(timerctl.next > timerctl.count) return;
    
    timer = timerctl.t0;
    for(;;){
        //timersのタイマはすべて動作中のタイマなので、flagsは見なくていい
        if(timer->timeout > timerctl.count) break;
        
        //timeout
        timer->flags = TIMER_FLAGS_ALLOC;
        fifo32_put(timer->fifo, timer->data);
        timer = timer->next;
    }
    
    //リスト先頭を更新
    timerctl.t0 = timer;
    timerctl.next = timerctl.t0->timeout;
}


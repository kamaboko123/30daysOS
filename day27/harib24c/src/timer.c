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
            timerctl.timers0[i].flags2 = 0;
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
    
    e = io_load_eflags(); //割り込みフラグを覚えておくため？
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

int timer_cancel(struct TIMER *timer){
    int e;
    struct TIMER *t;
    
    e = io_load_eflags();
    io_cli();
    
    //使用中
    if(timer->flags == TIMER_FLAGS_USING){
        //先頭(次のタイマを先頭にする)
        if(timer == timerctl.t0){
            t = timer->next;
            timerctl.t0 = t;
            timerctl.next = t->timeout;
        }
        else{
            t = timerctl.t0;
            
            //対象のタイマを探す
            for(;;){
                if(t->next == timer) break;
                t = t->next;
            }
            //t->nextは対象のタイマの前、つまりその次が対象のタイマ
            //対象のタイマをスキップさせるように次を指すようにする
            t->next = timer->next;
        }
        timer->flags = TIMER_FLAGS_ALLOC;
        io_store_eflags(e);
        
        //キャンセル成功
        return 1;
    }
    
    io_store_eflags(e);
    //キャンセル不要
    return 0;
}

void timer_cancelall(struct FIFO32 *fifo){
    int e;
    int i;
    struct TIMER *t;
    
    e = io_load_eflags();
    io_cli();
    for(i = 0; i < MAX_TIMER; i++){
        t = &timerctl.timers0[i];
        if(t->flags != 0 && t->flags2 != 0 && t->fifo == fifo){
            timer_cancel(t);
            timer_free(t);
        }
    }
    io_store_eflags(e);
}

void inthandler20(int *esp){
    int i, j;
    struct TIMER *timer;
    char ts = 0;
    
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
        if(timer != task_timer){
            fifo32_put(timer->fifo, timer->data);
        }
        else{
            ts = 1;
        }
        timer = timer->next;
    }
    
    //リスト先頭を更新
    timerctl.t0 = timer;
    timerctl.next = timerctl.t0->timeout;
    if(ts != 0){
        task_switch();
    }
}


#include "bootpack.h"

struct TASKCTL *taskctl;
struct TIMER *task_timer;

struct TASK *task_init(struct MEMMAN *memman){
    int i;
    struct TASK *task;
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
    taskctl = (struct TASKCTL *) memman_alloc_4k(memman, sizeof(struct TASKCTL));
    for(i = 0; i < MAX_TASKS; i++){
        taskctl->tasks0[i].flags = 0;
        taskctl->tasks0[i].sel = (TASK_GDT0 + i) * 8;
        set_segmdesc(gdt + TASK_GDT0 + i, 103, (int)&taskctl->tasks0[i].tss, AR_TSS32);
    }
    task = task_alloc();
    task->flags = 2; //動作中
    taskctl->running = 1;
    taskctl->now = 0;
    taskctl->tasks[0] = task;
    load_tr(task->sel);
    task_timer = timer_alloc();
    timer_settime(task_timer, 2);
    return task;
}

struct TASK *task_alloc(void){
    int i;
    struct TASK *task;
    
    for(i = 0; i < MAX_TASKS; i++){
        if(taskctl->tasks0[i].flags == 0){
            task = &taskctl->tasks0[i];
            
            task->flags = 1; //使用中
            task->tss.eflags = 0x00000202; //IF = 1
            task->tss.eax = 0;
            task->tss.ecx = 0;
            task->tss.edx = 0;
            task->tss.ebx = 0;
            task->tss.ebp = 0;
            task->tss.esi = 0;
            task->tss.edi = 0;
            task->tss.es = 0;
            task->tss.cs = 0;
            task->tss.ds = 0;
            task->tss.fs = 0;
            task->tss.gs = 0;
            task->tss.ldtr = 0;
            task->tss.iomap = 0x40000000;
            
            return task;
        }
    }
    
    return 0;
}

void task_run(struct TASK *task){
    task->flags = 2; //動作中
    taskctl->tasks[taskctl->running] = task; //動作中のタスクに加える
    taskctl->running++;
}

void task_switch(void){
    timer_settime(task_timer,  2);
    if(taskctl->running >= 2){
        taskctl->now++;
        if(taskctl->now == taskctl->running){
            taskctl->now = 0;
        }
        farjmp(0, taskctl->tasks[taskctl->now]->sel);
    }
}

void task_sleep(struct TASK *task){
    int i;
    char ts = 0;
    
    if(task->flags == 2){
        if(task == taskctl->tasks[taskctl->now]){
            //自分自身を寝かせる場合
            //処理が終わったあとに寝かせる必要があるのでフラグ立て
            ts = 1;
        }
        for(i = 0; i < taskctl->running; i++){
            //taskがどこにいるかを探す
            if(taskctl->tasks[i] == task) break;
        }
        
        taskctl->running--;
        if(i < taskctl->now){
            //管理中のタスクのリストで、寝かせる対象のタスクが、今実行中のタスクより前にある
            //対象のタスクを寝かせると、タスクが一つ減って、実行中のタスクのindexもずれる
            taskctl->now--;
        }
        
        for(; i < taskctl->running; i++){
            //ずらす
            taskctl->tasks[i] = taskctl->tasks[i + 1];
        }
        task->flags = 1; //動作停止中の状態
        //タスクスイッチ実行
        if(ts != 0){
            //もしnowが変な値になっていたら、修正する
            if(taskctl->now >= taskctl->running){
                taskctl->now = 0;
            }
            farjmp(0, taskctl->tasks[taskctl->now]->sel);
        }
    }
}

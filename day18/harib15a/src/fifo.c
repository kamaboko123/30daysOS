#include "bootpack.h"

void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf){
    fifo->size = size;
    fifo->buf;
    fifo->free = size;
    fifo->flags = 0;
    fifo->p = 0;
    fifo->q = 0;
}

int fifo8_put(struct FIFO8 *fifo, unsigned char data){
    if(fifo->free == 0) {
        fifo->flags |= FLAGS_OVERRUN;
        return -1;
    }
    
    fifo->buf[fifo->p] = data;
    fifo->p++;
    
    fifo->p %= fifo->size;
    fifo->free--;
}

int fifo8_get(struct FIFO8 *fifo){
    int data;
    
    if(fifo->free == fifo->size){
        return -1;
    }
    
    data = fifo->buf[fifo->q];
    fifo->q++;
    
    fifo->q %= fifo->size;
    fifo->free++;
    
    return data;
}

int fifo8_status(struct FIFO8 *fifo){
    return fifo->size - fifo->free;
}



void fifo32_init(struct FIFO32 *fifo, int size, int *buf, struct TASK *task){
    fifo->size = size;
    fifo->buf;
    fifo->free = size;
    fifo->flags = 0;
    fifo->p = 0;
    fifo->q = 0;
    
    fifo->task = task; //起こすタスク
}

int fifo32_put(struct FIFO32 *fifo, int data){
    if(fifo->free == 0) {
        fifo->flags |= FLAGS_OVERRUN;
        return -1;
    }
    
    fifo->buf[fifo->p] = data;
    fifo->p++;
    
    fifo->p %= fifo->size;
    fifo->free--;
    
    //タスクが寝てたら起こす
    if(fifo->task != 0){
        if(fifo->task->flags != 2){
            task_run(fifo->task, -1, 0);
        }
    }
}

int fifo32_get(struct FIFO32 *fifo){
    int data;
    
    if(fifo->free == fifo->size){
        return -1;
    }
    
    data = fifo->buf[fifo->q];
    fifo->q++;
    
    fifo->q %= fifo->size;
    fifo->free++;
    
    return data;
}

int fifo32_status(struct FIFO32 *fifo){
    return fifo->size - fifo->free;
}

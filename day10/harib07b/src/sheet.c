#include "bootpack.h"

struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize){
    struct SHTCTL *ctl;
    int i;
    ctl = (struct SHTCTL *) memman_alloc_4k(memman, sizeof(struct SHTCTL));
    
    if(ctl == 0){
        goto err;
    }
    
    ctl->vram = vram;
    ctl->xsize = xsize;
    ctl->ysize = ysize;
    
    ctl->top = -1; //1枚もない
    for(i = 0; i < MAX_SHEETS; i++) ctl->sheets0[i].flags = 0;
    
err:
    return ctl;
}

struct SHEET *sheet_alloc(struct SHTCTL *ctl){
    struct SHEET *sht;
    
    int i;
    for(i = 0; i < MAX_SHEETS; i++){
        if(ctl->sheets0[i].flags == 0){
            sht = &ctl->sheets0[i];
            sht->flags = SHEET_USE;
            sht->height = -1; //非表示
            
            return sht;
        }
        
    }
    
    return 0; //確保失敗
}

void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv){
    sht->buf = buf;
    sht->bxsize = xsize;
    sht->bysize = ysize;
    sht->col_inv = col_inv;
}

void sheet_updown(struct SHTCTL *ctl, struct SHEET *sht, int height){
    int h;
    int old = sht->height;
    
    //指定が高すぎたら一番上になるように修正
    if(height > ctl ->top){
        height = ctl->top + 1;
    }
    
    if(height < -1){
        height = -1;
    }
    
    sht->height = height;
    
    if(old > height){ //元よりも低くなる
        if(height >= 0){
            //間を引き上げる
            for(h = old; h > height; h--){
                ctl->sheets[h] = ctl->sheets[h-1];
                ctl->sheets[h]->height = h;
            }
            ctl->sheets[height] = sht;
        }
        else{ //非表示にする
            //上になっているものおろす
            if(ctl->top > old){ //元の位置よりも上シートが有る
                for(h = old; h < ctl->top; h++){
                    ctl->sheets[h] = ctl->sheets[h + 1];
                    ctl->sheets[h]->height = h;
                }
            }
            ctl->top--;
        }
    }
    
    //もとよりも高くなる
    else if(old < height){
        //元が非表示ではなかった
        if(old >= 0){
            //間のシートを引き下げる
            for(h = old; h < height; h++){
                ctl->sheets[h] = ctl->sheets[h + 1];
                ctl->sheets[h]->height = h;
            }
            ctl->sheets[height] = sht;
        }
        
        //非表示から表示状態へ
        else{
            //移動先を開けるために、topと挿入位置の間にあるシートを引き上げる
            for(h = ctl->top; h >= height; h--){
                ctl->sheets[h] = ctl->sheets[h + 1];
                ctl->sheets[h]->height = h;
            }
            
            ctl->sheets[height] = sht;
            ctl->top++;
        }
    }
    sheet_refresh(ctl);
}

void sheet_refresh(struct SHTCTL *ctl){
    int h;
    int bx;
    int by;
    int vx;
    int vy;
    
    unsigned char *buf;
    unsigned char c;
    unsigned char *vram = ctl->vram;
    
    struct SHEET *sht;
    
    for(h = 0; h <= ctl->top; h++){
        sht = ctl->sheets[h];
        buf = sht->buf;
        
        
        //描画(VRAMに書き込み)
        for(by = 0; by < sht->bysize; by++){
            vy = sht->vy0 + by;
            for(bx = 0; bx < sht->bxsize; bx++){
                vx = sht->vx0 + bx;
                c = buf[by * sht->bxsize + bx];
                if(c != sht->col_inv){
                    vram [vy * ctl->xsize + vx] = c;
                }
            }
        }
        
    }
}

void sheet_slide(struct SHTCTL *ctl, struct SHEET *sht, int vx0, int vy0){
    sht->vx0 = vx0;
    sht->vy0 = vy0;
    
    if(sht->height >= 0){
        sheet_refresh(ctl);
    }
}

void sheet_free(struct SHTCTL *ctl, struct SHEET *sht){
    if(sht->height >= 0){
        sheet_updown(ctl, sht, -1);
    }
    sht->flags = 0;
}

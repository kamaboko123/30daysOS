#include "bootpack.h"

struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize){
    struct SHTCTL *ctl;
    int i;
    ctl = (struct SHTCTL *) memman_alloc_4k(memman, sizeof(struct SHTCTL));
    
    if(ctl == 0){
        goto err;
    }
    
    ctl->map = (unsigned char *)memman_alloc_4k(memman, xsize * ysize);
    if(ctl->map == 0){
        memman_free_4k(memman, (int)ctl, sizeof(struct SHTCTL));
        goto err;
    }
    
    ctl->vram = vram;
    ctl->xsize = xsize;
    ctl->ysize = ysize;
    
    ctl->top = -1; //1枚もない
    for(i = 0; i < MAX_SHEETS; i++){
        ctl->sheets0[i].flags = 0;
        ctl->sheets0[i].ctl = ctl;
    }
    
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
            sht->task = 0; //自動で閉じない
            
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

void sheet_updown(struct SHEET *sht, int height){
    int h;
    int old = sht->height;
    struct SHTCTL *ctl = sht->ctl;
    
    
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
            sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize,  sht->vy0 + sht->bysize, height + 1);
            sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize,  sht->vy0 + sht->bysize, height + 1, old);
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
            sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize,  sht->vy0 + sht->bysize, 0);
            sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize,  sht->vy0 + sht->bysize, 0, old - 1);
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
                ctl->sheets[h + 1] = ctl->sheets[h];
                ctl->sheets[h + 1]->height = h + 1;
            }
            
            ctl->sheets[height] = sht;
            ctl->top++;
        }
        sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize,  sht->vy0 + sht->bysize, height);
        sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize,  sht->vy0 + sht->bysize, height, height);
    }
}

void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1){
    if(sht->height >= 0){ //表示中なら描き直す
        sheet_refreshsub(sht->ctl, sht->vx0 + bx0, sht->vy0 + by0, sht->vx0 + bx1, sht->vy0 + by1, sht->height, sht->height);
    }
}

void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0, int h1){
    int h;
    int bx;
    int by;
    int vx;
    int vy;
    int bx0;
    int by0;
    int bx1;
    int by1;
    int i;
    int i1;
    int *p;
    int *q;
    int *r;
    int sid4;
    int bx2;
    
    unsigned char *buf;
    unsigned char c;
    unsigned char *vram = ctl->vram;
    unsigned char *map = ctl->map;
    unsigned char sid;
    
    struct SHEET *sht;
    
    if(vx0 < 0) vx = 0;
    if(vy0 < 0) vy = 0;
    if(vx1 > ctl->xsize) vx1 = ctl->xsize;
    if(vy1 > ctl->ysize) vy1 = ctl->ysize;
    
    for(h = h0; h <= ctl->top; h++){
        sht = ctl->sheets[h];
        buf = sht->buf;
        sid = sht - ctl->sheets0;
        
        bx0 = vx0 - sht->vx0;
        by0 = vy0 - sht->vy0;
        bx1 = vx1 - sht->vx0;
        by1 = vy1 - sht->vy0;
        
        if(bx0 < 0) bx0 = 0;
        if(by0 < 0) by0 = 0;
        if(bx1 > sht->bxsize) bx1 = sht->bxsize;
        if(by1 > sht->bysize) by1 = sht->bysize;
        
        if((sht->vx0 & 3) == 0){
            //ウインドウの端のx座標が4の倍数
            i = (bx0 + 3) / 4; //bx0を4でったもの(端数切り上げ）
            i1 = bx1 / 4; //bx0を4で割ったもの（端数切り捨て）
            i1 = i1 - i;
            
            sid4 = sid | sid << 8 | sid << 16 | sid << 24;
            for(by = by0; by < by1; by++){
                vy = sht->vy0 + by;
                for(bx = bx0; bx < bx1 && (bx & 3) != 0; bx++){ //前の端数(1byteずつ)
                    vx = sht->vx0 + bx;
                    if(map[vy * ctl->xsize + vx] == sid){
                        vram[vy * ctl->xsize + vx] = buf[by * sht->bxsize + bx];
                    }
                }
                
                vx = sht->vx0 + bx;
                p = (int *) &map[vy * ctl->xsize + vx];
                q = (int *) &vram[vy * ctl->xsize + vx];
                r = (int *) &buf[by * sht->bxsize + bx];
                
                //4の倍数部分
                for(i = 0; i < i1; i++){
                    if(p[i] == sid4){
                        q[i] = r[i];
                    }
                    else{
                        bx2 = bx + i * 4;
                        vx = sht->vx0 + bx2;
                        
                        if(map[vy * ctl->xsize + vx + 0] == sid){
                            vram[vy * ctl->xsize + vx + 0] = buf[by * sht->bxsize + bx2 + 0];
                        }
                        if(map[vy * ctl->xsize + vx + 1] == sid){
                            vram[vy * ctl->xsize + vx + 1] = buf[by * sht->bxsize + bx2 + 1];
                        }
                        if(map[vy * ctl->xsize + vx + 2] == sid){
                            vram[vy * ctl->xsize + vx + 2] = buf[by * sht->bxsize + bx2 + 2];
                        }
                        if(map[vy * ctl->xsize + vx + 3] == sid){
                            vram[vy * ctl->xsize + vx + 3] = buf[by * sht->bxsize + bx2 + 3];
                        }
                    }
                }
                
                for(bx += i1 * 4; bx < bx1; bx++){ //後ろの端数(1byteずつ)
                    vx = sht->vx0 + bx;
                    if(map[vy * ctl->xsize + vx] == sid){
                        vram[vy * ctl->xsize + vx] = buf[by * sht->bxsize + bx];
                    }
                }
            }
        }
        else{ //1byte型
            for(by = by0; by < by1; by++){
                vy = sht->vy0 + by;
                for(bx = bx0; bx < bx1; bx++){
                    vx = sht->vx0 + bx;
                    if(map[vy * ctl->xsize + vx] == sid){
                        vram[vy * ctl->xsize + vx] = buf[by * sht->bxsize + bx];
                    }
                }
            }
        }
    }
}

void sheet_slide(struct SHEET *sht, int vx0, int vy0){
    int old_vx0 = sht->vx0;
    int old_vy0 = sht->vy0;
    
    sht->vx0 = vx0;
    sht->vy0 = vy0;
    struct SHTCTL *ctl = sht->ctl;
    
    if(sht->height >= 0){
        sheet_refreshmap(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 0);
        sheet_refreshmap(ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, sht->height);
        sheet_refreshsub(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 0, sht->height - 1);
        sheet_refreshsub(ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, sht->height, sht->height);
    }
}

void sheet_free(struct SHEET *sht){
    struct SHTCTL *ctl = sht->ctl;
    if(sht->height >= 0){
        sheet_updown(sht, -1);
    }
    sht->flags = 0;
}

void sheet_refreshmap(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0){
    int h;
    int bx;
    int by;
    int vx;
    int vy;
    int bx0;
    int by0;
    int bx1;
    int by1;
    int sid4;
    int *p;
    
    unsigned char *buf;
    unsigned char sid;
    unsigned char *map = ctl->map;
    
    struct SHEET *sht;
    
    if(vx0 < 0) vx0 = 0;
    if(vx0 < 0) vy0 = 0;
    
    if(vx1 > ctl->xsize) vx1 = ctl->xsize;
    if(vy1 > ctl->ysize) vy1 = ctl->ysize;
    
    for(h = h0; h <= ctl->top; h++){
        sht = ctl->sheets[h];
        sid = sht - ctl->sheets0;
        
        buf = sht->buf;
        bx0 = vx0 - sht->vx0;
        by0 = vy0 - sht->vy0;
        bx1 = vx1 - sht->vx0;
        by1 = vy1 - sht->vy0;
        
        if(bx0 < 0) bx0 = 0;
        if(by0 < 0) by0 = 0;
        
        if(bx1 > sht->bxsize) bx1 = sht->bxsize;
        if(by1 > sht->bysize) by1 = sht->bysize;
        
        if(sht->col_inv == -1){
            //透明色がない場合、書くピクセルの判定をせずに描画することで高速化
            //4byteずつ書き込むとさらに高速化できる
            if((sht->vx0 & 3) == 0 && (bx0 & 3) == 0 && (bx1 & 3) == 0){
                bx1 = (bx1 - bx0) / 4; //mov回数(4byteずつmovするので)
                
                sid4 = sid | sid << 8 | sid << 16 | sid << 24;
                for(by = by0; by < by1; by++){
                    vy = sht->vy0 + by;
                    vx = sht->vx0 + bx0;
                    p = (int *) &map[vy * ctl->xsize + vx];
                    for(bx = 0; bx < bx1; bx++){
                        p[bx] = sid4;
                    }
                }
            }
            else{
                for(by = 0; by < by1; by++){
                    vy = sht->vy0 + by;
                    for(bx = bx0; bx < bx1; bx++){
                        vx = sht->vx0 + bx;
                        map[vy * ctl->xsize + vx] = sid;
                    }
                }
            }
        }
        else{
            //透明色あり
            for(by = by0; by < by1; by++){
                vy = sht->vy0 + by;
                for(bx = bx0; bx < bx1; bx++){
                    vx = sht->vx0 + bx;
                    if(buf[by * sht->bxsize + bx] != sht->col_inv){
                        map[vy * ctl->xsize + vx] = sid;
                    }
                }
            }
        }
    }
}


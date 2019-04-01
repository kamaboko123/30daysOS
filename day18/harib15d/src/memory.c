#include "bootpack.h"

unsigned int memtest(unsigned int start, unsigned int end){
    char flg486 = 0;
    unsigned int eflg;
    unsigned int cr0;
    unsigned int i;
    
    //486以降ならキャッシュを無効にする必要がある
    
    //386か486かを確認する
    //EFLAGSを読み込んで、ac-bit(第18bit)を1にしてEFLAGSに戻す
    //386ではac-bitを1にしても自動的に0に戻ってしまうので、これで386か486かを判定できる
    eflg = io_load_eflags();
    eflg |= EFLAGS_AC_BIT;
    io_store_eflags(eflg);
    eflg = io_load_eflags();
    if((eflg & EFLAGS_AC_BIT) != 0) flg486 = 1;
    
    //ac-bitは0に戻しておく
    eflg &= ~EFLAGS_AC_BIT;
    io_store_eflags(eflg);
    
    //486以降ならキャッシュを無効にする
    //crにあるのフラグで設定できる
    if(flg486 != 0){
        cr0 = load_cr0();
        cr0 |= CR0_CACHE_DISABLE;
        store_cr0(cr0);
    }
    
    i = memtest_sub(start, end);
    
    //キャッシュを有効に戻す
    if(flg486 != 0){
        cr0 = load_cr0();
        cr0 &= ~CR0_CACHE_DISABLE;
        store_cr0(cr0);
    }
    
    return i;
}


void memman_init(struct MEMMAN *man){
    man->frees = 0;
    man->maxfrees = 0;
    man->lostsize = 0;
    man->losts = 0;
}

unsigned int memman_total(struct MEMMAN *man){
    unsigned int i;
    unsigned int t = 0;
    
    for(i = 0; i < man->frees; i++){
        t += man->free[i].size;
    }
    
    return t;
}


unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size){
    unsigned int a;
    size = (size + 0x0fff) & 0xfffff000;
    a = memman_alloc(man, size);
    
    return a;
}

int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size){
    int i;
    size = (size + 0x0fff) & 0xfffff000;
    i = memman_free(man, addr, size);
    
    return i;
}

unsigned int memman_alloc(struct MEMMAN *man, unsigned int size){
    unsigned int i;
    unsigned int a;
    
    for(i = 0; i < man->frees; i++){
        if(man->free[i].size >= size){
            //確保可能
            a = man->free[i].addr;
            man->free[i].addr += size;
            man->free[i].size -= size;
            
            //free[i]がなくなったので前へ詰める
            if(man->free[i].size == 0){
                man->frees--;
                for(; i < man->frees; i++){
                    man->free[i] = man->free[i + 1];
                }
            }
            
            return a;
        }
    }
    
    return 0;
}

int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size){
    int i;
    int j;
    
    //挿入場所を決める（freeがaddr順に並んでいたほうがまとめやすい）
    for(i = 0; i < man->frees; i++){
        if(man->free[i].addr > addr) break;
    }
    
    // free[i - 1].addr < addr < free[i]
    if(i > 0){
        //iが0より大きい＝前がある
        
        //まとめられる
        if(man->free[i - 1].addr + man->free[i - 1].size == addr){
            man->free[i - 1].size += size;
            
            //freesより小さい、後ろもある
            if(i < man->frees){
                
                //後ろもまとめられる
                if(addr + size == man->free[i].addr){
                    man->free[i-1].size += man->free[i].size;
                    
                    //後ろをまとめてfree[i]が空いたのでつめる
                    man->frees--;
                    for(; i < man->frees; i++){
                        man->free[i] = man->free[i + 1];
                    }
                }
            }
            
            return 0;
        }
    }
    
    //後ろがある
    if(i < man->frees){
        //まとめられる
        if(addr + size == man->free[i].addr){
            man->free[i].addr = addr;
            man->free[i].size += size;
            
            return 0;
        }
    }
    
    //以下まとめられない場合
    
    //リストに追加可能
    if(man->frees < MEMMAN_FREES){
        //リストに追加するために、後ろへつめる
        for(j = man->frees; j > i; j--){
            man->free[j] = man->free[j - 1];
        }
        //追加
        man->frees++;
        if(man->maxfrees < man->frees){
            man->maxfrees = man->frees; //最大値更新
        }
        
        man->free[i].addr = addr;
        man->free[i].size = size;
        return 0;
    }
    
    //リストに追加できない（エラー）
    man->losts++;
    man->lostsize += size;
    
    return -1;
}


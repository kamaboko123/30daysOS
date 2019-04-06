#include "bootpack.h"

void file_readfat(int *fat, unsigned char *img){
    int i;
    int j = 0;
    
    //2880 byte -> fatの大きさ
    for(i = 0; i < 2880; i += 2){
        fat[i + 0] = (img[j + 0] | img[j + 1] << 8) & 0xfff;
        fat[i + 1] = (img[j + 1]  >> 4 | img[j + 2] << 4) & 0xfff;
        j += 3;
    }
}

void file_loadfile(int clustono, int size, char *buf, int *fat, char *img){
    int i;
    
    for(;;){
        if(size <= 512){
            for(i = 0; i < size; i++){
                buf[i] = img[clustono * 512 + i];
            }
            break;
        }
        
        for(i = 0; i < 512; i++){
            buf[i] = img[clustono * 512 + i];
        }
        
        size -= 512;
        buf += 512;
        clustono = fat[clustono];
    }
}


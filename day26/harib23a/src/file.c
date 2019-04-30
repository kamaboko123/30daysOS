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

struct FILEINFO *file_search(char *name, struct FILEINFO *finfo, int max){
    int i, j;
    char s[12];
    for(j = 0; j < 11; j++){
        s[j] = ' ';
    }
    j = 0;
    
    //nameをFATの情報に揃える
    for(i = 0; name[i] != 0; i++){
        if(j >= 11) return 0; //見つからなかった
        if(name[i] == '.' && j <= 8){
            j = 8; //これ以降は拡張子
        }
        else{
            s[j] = name[i];
            if(s[j] >= 'a' && s[j] <= 'z'){
                s[j] -= 0x20;
            }
            j++;
        }
    }
    
    for(i = 0; i < max;){
        if(finfo[i].name[0] == 0x00) break;
        if((finfo[i].type & 0x18) == 0){
            for(j = 0; j < 11; j++){
                if(finfo[i].name[j] != s[j]) goto next;
            }
            return finfo + i;
        }
next:
        i++;
    }
    
    return 0; //見つからなかった
}

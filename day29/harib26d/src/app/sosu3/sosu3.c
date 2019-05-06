#include "apilib.h"
#include "stdlibc.h"

#define MAX 10000

void HariMain(void){
    char *flag;
    char s[8];
    int i, j;
    
    api_initmalloc();
    flag = api_malloc(MAX);
    
    for(i = 0; i < MAX; i++){
        flag[i] = 0;
    }
    for(i = 2; i < MAX; i++){
        if(flag[i] == 0){
            _sprintf(s, "%d ", i);
            api_putstr0(s);
            for(j = i * 2; j < MAX; j += i){
                flag[j] = 1;
            }
        }
    }
    
    api_end();
}

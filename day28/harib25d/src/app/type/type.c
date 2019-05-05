#include "apilib.h"

void HariMain(void){
    int fh;
    char c;
    char cmdline[30];
    char *p;
    
    api_cmdline(cmdline, 30);
    //スペースが来るまで読み飛ばす
    for(p = cmdline; *p > ' '; p++);
    //スペースを読み飛ばす
    for(; *p == ' '; p++);
    
    fh = api_fopen(p);
    if(fh != 0){
        for(;;){
            if(api_fread(&c, 1, fh) == 0) break;
            api_putchar(c);
        }
    }
    else{
        api_putstr0("File not found\n");
    }
    api_end();
}

#include "apilib.h"
#include "stdlibc.h"

void HariMain(void){
    int i;
    int timer;
    
    timer = api_alloctimer();
    api_inittimer(timer, 128);
    
    for(i = 2000000; i >= 2000; i -= i / 100){
        //20KHz - 20Hz
        //1%ずつ減らす
        api_beep(i);
        api_settimer(timer, 1);
        if(api_getkey(1) != 128){
            break;
        }
    }
    
    api_beep(0);
    api_end();
}

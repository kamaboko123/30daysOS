#include "apilib.h"
#include "stdlibc.h"

void HariMain(void){
    char *buf;
    int win;
    int i;
    int x, y;
    
    api_initmalloc();
    buf = api_malloc(160 * 100);
    win = api_openwin(buf, 160, 100, -1, "walk");
    
    api_boxfillwin(win, 4, 24, 155, 95, 0);
    x = 76;
    y = 56;
    api_putstrwin(win, x, y, 3, 1, "*");
    
    for(;;){
        i = api_getkey(1);
        api_putstrwin(win, x, y, 0, 1, "*"); //黒で消す
        if(i == '4' && x > 4) x -= 8;
        if(i == '6' && x < 148) x += 8;
        if(i == '8' && y > 24) y -= 8;
        if(i == '2' && y < 80) y += 8;
        if(i == 0x0a) break; //enter
        api_putstrwin(win, x, y, 3, 1, "*");
    }
    api_closewin(win);
    api_end();
}

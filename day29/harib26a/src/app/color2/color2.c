#include "apilib.h"
#include "stdlibc.h"

unsigned char rgb2pal(int r, int g, int b, int x, int y);

void HariMain(void){
    char *buf;
    int win;
    int x, y;
    int r, g, b;
    
    api_initmalloc();
    buf = api_malloc(144 * 164);
    win = api_openwin(buf, 144, 164, -1, "color");
    
    for(y = 0; y < 128; y++){
        for(x = 0; x < 128; x++){
            r = x * 2;
            g = y * 2;
            b = 0;
            buf[(x + 8) + (y + 28) * 144] = rgb2pal(x * 2, y *2, 0, x, y);
        }
    }
    api_refreshwin(win, 8, 28, 136, 156);
    api_getkey(1);
    api_closewin(win);
    api_end();
}

unsigned char rgb2pal(int r, int g, int b, int x, int y){
    static int table[4] = {3, 1, 0, 2};
    int i;
    
    x &= 1; //奇数か偶数か
    y &= 1;
    
    //中間色を作るための定数
    i = table[x + y * 2];
    r = (r * 21) / 256; //0-20
    g = (g * 21) / 256;
    b = (b * 21) / 256;
    r = (r + i) / 4; //0-5
    g = (g + i) / 4;
    b = (b + i) / 4;
    
    return 16 + r + g * 6 + b * 36;
}

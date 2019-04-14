#include "haribote.h"

void HariMain(void){
    *((char *) 0x00102600) = 0;
    api_end();
}

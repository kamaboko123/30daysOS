/*
This linker script is created by refer following page.
http://takeisamemo.blogspot.com/2014/09/os30os-3-7.html
*/

void io_hlt(void);

void HariMain(void){
fin:
    io_hlt();
    goto fin;
}

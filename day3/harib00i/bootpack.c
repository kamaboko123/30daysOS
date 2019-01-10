/*
This linker script is created by refer following page.
http://takeisamemo.blogspot.com/2014/09/os30os-3-7.html
*/

void HariMain(void){
fin:
    __asm__("hlt\n\t");
    goto fin;
}

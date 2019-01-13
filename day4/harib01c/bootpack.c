/*
This linker script is created by refer following page.
http://takeisamemo.blogspot.com/2014/09/os30os-3-7.html
*/

void io_hlt(void);

void HariMain(void){
    //char *p;
    int i;
    
    for(i = 0xa0000; i <= 0xaffff; i++){
        //p = (char *)i;
        //*p = i & 0x0f;
        
        *(char *)i = i & 0x0f;
    }
    
    for(;;){
        io_hlt();
    }
}

/*
This linker script is created by refer following page.
http://takeisamemo.blogspot.com/2014/09/os30os-3-7.html
*/

void io_hlt(void);
void write_mem8(int addr, int data);

void HariMain(void){
    int i;
    
    for(i = 0xa0000; i <= 0xaffff; i++){
        write_mem8(i, i & 0x0f);
    }
    
    for(;;){
        io_hlt();
    }
}

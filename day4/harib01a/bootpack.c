void io_hlt(void);
void write_mem8(int addr, int data);

void HariMain(void){
    int i;
    
    for(i = 0x0000; i <= 0xaffff; i++){
        write_mem8(i, 10);
    }
    
    for(;;){
        io_hlt();
    }
}

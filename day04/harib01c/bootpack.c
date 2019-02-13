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

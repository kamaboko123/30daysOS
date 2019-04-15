#include "bootpack.h"

void init_gdtidt(void){
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
    struct GATE_DESCRIPTOR *idt = (struct GATE_DESCRIPTOR *) ADR_IDT;
    int i;
    
    for( i = 0; i < LIMIT_GDT / sizeof(struct SEGMENT_DESCRIPTOR); i++){
        set_segmdesc(gdt + i, 0, 0, 0);
    }
    
    //全メモリ
    set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, AR_DATA32_RW);
    //bootpack用
    set_segmdesc(gdt + 2, LIMIT_BOTPAK, 0x00280000, AR_CODE32_ER);
    
    load_gdtr(LIMIT_GDT, ADR_GDT);
    
    for(i = 0; i < LIMIT_IDT / sizeof(struct GATE_DESCRIPTOR); i++){
        set_gatedesc(idt + i, 0, 0, 0);
    }
    load_idtr(LIMIT_IDT, ADR_IDT);
    
    set_gatedesc(idt + 0x0d, (int)asm_inthandler0d, 2 << 3, AR_INTGATE32);
    set_gatedesc(idt + 0x20, (int)asm_inthandler20, 2 << 3, AR_INTGATE32);
    set_gatedesc(idt + 0x21, (int)asm_inthandler21, 2 << 3, AR_INTGATE32);
    set_gatedesc(idt + 0x27, (int)asm_inthandler27, 2 << 3, AR_INTGATE32);
    set_gatedesc(idt + 0x2c, (int)asm_inthandler2c, 2 << 3, AR_INTGATE32);
    set_gatedesc(idt + 0x40, (int)asm_hrb_api, 2 << 3, AR_INTGATE32 + 0x60); //アプリに使わせても良い(0x60を足す)
}

void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar){
    if(limit > 0xffffff){
        //4Kページとして扱う
        ar = ar | 0x8000; //G_bit = 1
        limit = limit / 0x1000;
    }
    
    //下位16bit
    sd->limit_low = limit & 0xffff;
    sd->base_low = base & 0xffff;
    
    //中位8bit
    sd->base_mid = (base >> 16) & 0xff;
    //ar下位8bit
    sd->access_right = ar & 0xff;
    
    //下位8bit
    //limit_highの上位4bitはarの拡張アクセス権
    sd->limit_high = (((limit >> 16) & 0x0f) | (ar >> 8) & 0xf0);
    sd->base_high = (base >> 24) & 0xff;
}

void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar){
    gd->offset_low = offset & 0xffff;
    gd->selector = selector;
    gd->dw_count = (ar >> 8) & 0xff;
    gd->access_right = ar & 0xff;
    gd->offset_high = (offset >> 16) & 0xffff;
}


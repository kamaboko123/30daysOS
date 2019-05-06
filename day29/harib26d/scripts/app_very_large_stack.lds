/*
This linker script is created by refer following page.
https://vanya.jp.net/os/haribote.html
*/

/*
64KB
*/

OUTPUT_FORMAT("binary");

SECTIONS
{
    .head 0x0 : {
        LONG(128 * 1024)      /*  0 : stack+.data+heap の大きさ（4KBの倍数） */
        LONG(0x69726148)      /*  4 : シグネチャ "Hari" */
        LONG(0)               /*  8 : mmarea の大きさ（4KBの倍数） */
        LONG(0x10000)          /* 12 : スタック初期値＆.data転送先 */
        LONG(SIZEOF(.data))   /* 16 : .dataサイズ */
        LONG(LOADADDR(.data)) /* 20 : .dataの初期値列のファイル位置 */
        LONG(0xE9000000)      /* 24 : 0xE9000000 */
        LONG(HariMain - 0x20) /* 28 : エントリアドレス - 0x20 */
        LONG(24*1024)         /* 32 : heap領域（malloc領域）開始アドレス */
    }
    
    .text : { *(.text) }
    
    .data 0x10000 : AT ( ADDR(.text) + SIZEOF(.text) ) {
        *(.data)
        *(.rodata*)
        *(.bss)
    }
    
    /DISCARD/ : { *(.eh_frame) }
}

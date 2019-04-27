#ifndef BOOTPACK_H
#define BOOTPACK_H

#include <stdarg.h>
#include "stdlibc.h"

#define ADR_BOOTINFO 0x00000ff0

struct BOOTINFO{
    char cyls;
    char leds;
    char vmode;
    char reserve;
    short scrnx;
    short scrny;
    char *vram;
};

struct MOUSE_DEC{
    unsigned char buf[3];
    unsigned char phase; //decode phase
    int x;
    int y;
    int btn;
};

//memory.c
#define EFLAGS_AC_BIT 0x00040000 //18(AC) bit
#define CR0_CACHE_DISABLE 0x60000000 //30(CD), 21(PG) bit
#define MEMMAN_ADDR 0x003c0000 //メモリ管理情報を置くアドレス
#define MEMMAN_FREES 4090 //メモリの空き領域リストの最大個数

struct FREEINFO{
    unsigned int addr;
    unsigned int size;
};

struct MEMMAN{
    int frees; //空き情報の個数
    int maxfrees; //freesの最大値
    int lostsize; //解放に失敗した合計サイズ
    int losts; //解放に失敗した回数
    struct FREEINFO free[MEMMAN_FREES];
};

unsigned int memtest(unsigned int start, unsigned int end);
unsigned int memtest_sub(unsigned int start, unsigned int end);
void memman_init(struct MEMMAN *man);
unsigned int memman_total(struct MEMMAN *man);
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);
unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size);
int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size);


//asm_func.s
void io_hlt(void);
void io_cli(void);
void io_sti(void);
void io_stihlt(void);
void io_out8(int port, int data);
int io_in8(int port);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void asm_inthandler21(void);
void asm_inthandler2c(void);
void asm_inthandler27(void);
void asm_inthandler20(void);
int load_cr0(void);
void store_cr0(int cr0);
void load_tr(int tr);
void farjmp(int eip, int cs);

//dsctbl.c
#define ADR_IDT 0x0026f800
#define LIMIT_IDT 0x000007ff
#define ADR_GDT 0x00270000
#define LIMIT_GDT 0x0000ffff

//asmheadの中でbootpackは0x00280000 - 0x002ffffにコピーされる
#define ADR_BOTPAK 0x00280000
#define LIMIT_BOTPAK  0x0007ffff

//上位4bitはGD00で、GはG bit, Dはセグメントモード(1=32bit, 0=16bit)を表す
//16bitモードは80286互換のためで、bios呼び出しとかでは使えない、なので通常はD=1で使う
//下位は、
//0x00 : 未使用
//0x92 : システム、読み書き
//0x9a : システム、読み込み、実行
//0xf2 : アプリケーション、読み書き
//0xfa : アプリケーション、読み込み、実行
#define AR_DATA32_RW  0x4092
#define AR_CODE32_ER  0x409a
#define AR_INTGATE32  0x008e
#define AR_TSS32 0x0089

struct SEGMENT_DESCRIPTOR{
    short limit_low, base_low;
    char base_mid, access_right;
    char limit_high, base_high;
};

struct GATE_DESCRIPTOR{
    short offset_low, selector;
    char dw_count, access_right;
    short offset_high;
};

void init_gdtidt(void);

void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);

void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);


//graphic.c
#define COL8_000000 0
#define COL8_FF0000 1
#define COL8_00FF00 2
#define COL8_FFFF00 3
#define COL8_0000FF 4
#define COL8_FF00FF 5
#define COL8_00FFFF 6
#define COL8_FFFFFF 7
#define COL8_C6C6C6 8
#define COL8_840000 9
#define COL8_008400 10
#define COL8_848400 11
#define COL8_000084 12
#define COL8_840084 13
#define COL8_008484 14
#define COL8_848484 15

void init_palette(void);
void init_screen8(char *vram, int xsize, int ysize);
void set_palette(int start, int end, unsigned char *rgb);
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s);
void putfont8(char *vram, int xsize, int x, int y, char c, char *font);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
void init_mouse_cursor8(char *mouse, char bc);
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize);

//int.c
//PICのポート番号
#define PIC0_ICW1 0x0020
#define PIC0_OCW2 0x0020
#define PIC0_IMR  0x0021
#define PIC0_ICW2 0x0021
#define PIC0_ICW3 0x0021
#define PIC0_ICW4 0x0021
#define PIC1_ICW1 0x00a0
#define PIC1_OCW2 0x00a0
#define PIC1_IMR  0x00a1
#define PIC1_ICW2 0x00a1
#define PIC1_ICW3 0x00a1
#define PIC1_ICW4 0x00a1

#define PORT_KEYDAT 0x0060
#define PORT_KEYSTA 0x0064
#define PORT_KEYCMD 0x0064
#define KEYSTA_SEND_NOTREADY 0x02
#define KEYCMD_WRITE_MODE 0x60
#define KBC_MODE 0x47
#define KEYCMD_SENDTO_MOUSE 0xd4
#define MOUSECMD_ENABLE 0xf4

#define KEYBUF_SIZE 32
#define MOUSEBUF_SIZE 128

void init_pic(void);
void inthandler27(int *esp);

//fifo.c
#define FLAGS_OVERRUN 0x0001

struct FIFO8{
    unsigned char *buf;
    int p; //write
    int q; //read
    int size;
    int free;
    int flags; //overrun
};

void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf);
int fifo8_put(struct FIFO8 *fifo, unsigned char data);
int fifo8_get(struct FIFO8 *fifo);
int fifo8_status(struct FIFO8 *fifo);

struct FIFO32{
    int *buf;
    int p; //write
    int q; //read
    int size;
    int free;
    int flags; //overrun
    struct TASK *task;
};

void fifo32_init(struct FIFO32 *fifo, int size, int *buf, struct TASK *task);
int fifo32_put(struct FIFO32 *fifo, int data);
int fifo32_get(struct FIFO32 *fifo);
int fifo32_status(struct FIFO32 *fifo);



//keyboard.c
extern struct FIFO32 *keyfifo;
extern int keydata0;

void inthandler21(int *esp);
void init_keyboard(struct FIFO32 *fifo, int data0);
void wait_KBC_sendready(void);


//mouse.c
extern struct FIFO32 *mousefifo;
extern int mousedata0;
void inthandler2c(int *esp);
void enable_mouse(struct FIFO32 *fifo, int data0, struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);


//sheet.c
#define MAX_SHEETS 256
#define SHEET_USE 1

struct SHEET{
    unsigned char *buf;
    int bxsize; //大きさ
    int bysize;
    int vx0; //位置
    int vy0;
    int col_inv; //透明色
    int height; //高さ
    int flags;
    
    struct SHTCTL *ctl;
};

struct SHTCTL{
    unsigned char *vram;
    unsigned char *map;
    int xsize;
    int ysize;
    int top; //一番上のSHEETの高さ
    struct SHEET *sheets[MAX_SHEETS];
    struct SHEET sheets0[MAX_SHEETS];
};

struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize);
struct SHEET *sheet_alloc(struct SHTCTL *ctl);
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv);
void sheet_updown(struct SHEET *sht, int height);
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1);
void sheet_slide(struct SHEET *sht, int vx0, int vy0);
void sheet_free(struct SHEET *sht);
void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0, int h1);
void sheet_refreshmap(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0);


//timer.c
#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040

#define MAX_TIMER 500

#define TIMER_FLAGS_ALLOC 1
#define TIMER_FLAGS_USING 2

struct TIMER{
    struct TIMER *next;
    unsigned int timeout;
    unsigned int flags;
    struct FIFO32 *fifo;
    int data;
};

struct TIMERCTL{
    unsigned int count;
    unsigned int next;
    struct TIMER *t0;
    struct TIMER timers0[MAX_TIMER];
};

extern struct TIMERCTL timerctl;

void init_pit(void);
struct TIMER *timer_alloc(void);
void timer_free(struct TIMER *timer);
void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data);
void timer_settime(struct TIMER *timer, unsigned int timeout);
void inthandler20(int *esp);

//bootpack.c
void make_window8(unsigned char *buf, int xsize, int ysize, char *title, char act);
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l);
void task_b_main(struct SHEET *sht_back);

//mtask.c
#define MAX_TASKS 1000 //最大タスク数
#define TASK_GDT0 3 //タスクに割り当てるGDTの最初の位置

//task state segment
struct TSS32{
    int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
    int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
    int es, cs, ss, ds, fs, gs;
    int ldtr, iomap;
};

struct TASK{
    int sel; //GDT番号
    int flags;
    int priority;
    struct TSS32 tss;
};

struct TASKCTL{
    int running; //動作中のタスクの数
    int now; //実行中のタスク
    struct TASK *tasks[MAX_TASKS];
    struct TASK tasks0[MAX_TASKS];
};

extern struct TASKCTL *taskctl;
extern struct TIMER *task_timer;

struct TASK *task_init(struct MEMMAN *memman);
struct TASK *task_alloc(void);
void task_run(struct TASK *task, int priority);
void task_switch(void);
void task_sleep(struct TASK *task);
#endif

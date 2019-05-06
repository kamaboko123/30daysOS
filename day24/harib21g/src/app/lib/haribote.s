.arch i486

.extern HariMain
.global __start
__start:
    call HariMain
    lret

.global api_putchar
#void api_putchar(int c)
api_putchar:
    movl $1, %edx
    movb 4(%esp), %al
    int $0x40
    ret

.global api_putstr0
#void api_putstr0(char *s)
api_putstr0:
    #cdeclではebxは呼び出し前後で変わってはいけない事になっているので、保存して戻す
    push %ebx
    movl $2, %edx
    movl 8(%esp), %ebx
    int $0x40
    pop %ebx
    ret

#void api_end(void)
.global api_end
api_end:
    movl $4, %edx
    int $0x40


#int api_openwin(char *buf, int xsiz, int ysiz, int col_inv, char *title)
.global api_openwin
api_openwin:
    push %edi
    push %esi
    push %ebx
    movl $5, %edx
    movl 16(%esp), %ebx #buf
    movl 20(%esp), %esi #xsiz
    movl 24(%esp), %edi #ysiz
    movl 28(%esp), %eax #col_inv
    movl 32(%esp), %ecx #title
    int $0x40
    pop %ebx
    pop %esi
    pop %edi
    ret

#void putstrwin(int win, int x, int y, int col, int len, char *str)
.global api_putstrwin
api_putstrwin:
    push %edi
    push %esi
    push %ebp
    push %ebx
    movl $6, %edx
    movl 20(%esp), %ebx #win
    movl 24(%esp), %esi #x
    movl 28(%esp), %edi #y
    movl 32(%esp), %eax #col
    movl 36(%esp), %ecx #len
    movl 40(%esp), %ebp #str
    int $0x40
    pop %ebx
    pop %ebp
    pop %esi
    pop %edi
    ret


#void boxfillwin(int win, int x0, int y0, int x1, int y1, int col)
.global api_boxfillwin
api_boxfillwin:
    push %edi
    push %esi
    push %ebp
    push %ebx
    movl $7, %edx
    movl 20(%esp), %ebx #win
    movl 24(%esp), %eax #x0
    movl 28(%esp), %ecx #y0
    movl 32(%esp), %esi #x1
    movl 36(%esp), %edi #y1
    movl 40(%esp), %ebp #str
    int $0x40
    pop %ebx
    pop %ebp
    pop %esi
    pop %edi
    ret

#void api_initmalloc(void)
.global api_initmalloc
api_initmalloc:
    push %ebx
    movl $8, %edx
    movl %cs:(0x0020), %ebx #malloc領域
    movl %ebx, %eax
    addl $32 * 1024, %eax #32KBを足す
    movl %cs:(0x0000), %ecx #データセグメントの大きさ
    sub %eax, %ecx
    int $0x40
    pop %ebx
    ret

#char *api_malloc(int size)
.global api_malloc
api_malloc:
    push %ebx
    movl $9, %edx
    movl %cs:(0x0020), %ebx
    movl 8(%esp), %ecx #size
    int $0x40
    pop %ebx
    ret

#void api_free(char *addr, int size)
.global api_free
api_free:
    push %ebx
    movl $10, %edx
    movl %cs:(0x0020), %ebx
    movl 8(%esp), %eax #addr
    movl 12(%esp), %ecx #size
    int $0x40
    pop %ebx
    ret

#void api_point(int win, int x, int y, int col)
.global api_point
api_point:
    push %edi
    push %esi
    push %ebx
    movl $11, %edx
    movl 16(%esp), %ebx #win
    movl 20(%esp), %esi #x
    movl 24(%esp), %edi #y
    movl 28(%esp), %eax #col
    int $0x40
    pop %ebx
    pop %esi
    pop %edi
    ret

#void api_refreshwin(int win, int x0, int y0, int x1, int y1)
.global api_refreshwin
api_refreshwin:
    push %edi
    push %esi
    push %ebx
    movl $12, %edx
    movl 16(%esp), %ebx #win
    movl 20(%esp), %eax #x0
    movl 24(%esp), %ecx #y0
    movl 28(%esp), %esi #x1
    movl 32(%esp), %edi #y1
    int $0x40
    pop %ebx
    pop %esi
    pop %edi
    ret

#void api_linewin(int win, int x0, int y0, int x1, int y1, int col)
.global api_linewin
api_linewin:
    push %edi
    push %esi
    push %ebp
    push %ebx
    movl $13, %edx
    movl 20(%esp), %ebx #win
    movl 24(%esp), %eax #x0
    movl 28(%esp), %ecx #y0
    movl 32(%esp), %esi #x1
    movl 36(%esp), %edi #y1
    movl 40(%esp), %ebp #col
    int $0x40
    pop %ebx
    pop %ebp
    pop %esi
    pop %edi
    ret

#void api_closewin(int win)
.global api_closewin
api_closewin:
    push %ebx
    mov $14, %edx
    movl 8(%esp), %ebx #win
    int $0x40
    pop %ebx

#int api_getkey(int mode)
.global api_getkey
api_getkey:
    movl $15, %edx
    movl 4(%esp), %eax #mode
    int $0x40
    ret

#int api_alloctimer(void)
.global api_alloctimer
api_alloctimer:
    movl $16, %edx
    int $0x40
    ret

#void api_inittimer(int timer, int data)
.global api_inittimer
api_inittimer:
    push %ebx
    movl $17, %edx
    movl 8(%esp), %ebx  #timer
    movl 12(%esp), %eax #data
    int $0x40
    pop %ebx
    ret

#void api_settimer(int timer, int time)
.global api_settimer
api_settimer:
    push %ebx
    movl $18, %edx
    movl 8(%esp), %ebx  #imer
    movl 12(%esp), %eax #time
    int $0x40
    pop %ebx
    ret

#void api_freetimer(int timer)
.global api_freetimer
api_freetimer:
    push %ebx
    movl $19, %edx
    movl 8(%esp), %ebx  #imer
    int $0x40
    pop %ebx
    ret



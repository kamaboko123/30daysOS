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
    movl 29(%esp), %eax #col_inv
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


#boxfillwin(int win, int x0, int y0, int x1, int y1, int col)
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



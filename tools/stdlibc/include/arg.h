#ifndef ARG_H
#define ARG_H

typedef char * my_va_list;

//4byte単位にする
//4-1(=3)を足して、下2bitを切り下げる(3=00000011をビット反転させてAND)
#define _arg_size(x) \
    ((sizeof(x) + 0x03) & ~0x03)

#define my_va_start(arg_p, last) \
    arg_p = (char *)((char *)&last + _arg_size(last))

#define my_va_arg(arg_p, type) \
   (*(type *)((arg_p += _arg_size(type)) - _arg_size(type)))

#define my_va_end(x)

#endif

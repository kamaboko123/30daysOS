#include "bootpack.h"

void *sprintf(char *s, char *format, ...){
    va_list args;
    int length = 0;
    
    va_start(args, format);
    
    while(*format != '\0'){
        if(*format == '%'){
            format++;
            
            if(*format == 'd'){
                s += to_dec_asc(s, va_arg(args, int));
            }
            if(*format == 'x'){
                s += to_hex_asc(s, va_arg(args, int));
            }
        }
        else{
          *s = *format;
          s++;
        }
        format++;
    }
    
    va_end(args);
}

unsigned int to_dec_asc(char *buf, int n){
    char *p;
    unsigned int ret;
    unsigned int i;
    
    i = ndigit(n);
    ret = i;
    p = buf;
    
    while(i > 0){
        *p = ((n / upow(10, i - 1)) % 10) + '0';
        p++;
        i--;
    }
    return(ret);
}

unsigned int to_hex_asc(char *buf, int n){
    char *p;
    unsigned int ret;
    unsigned int i;
    char charset[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    
    i = ndigit(n);
    ret = i;
    p = buf;
    
    while(i > 0){
        *p = charset[((n / upow(16, i - 1)) % 16)];
        
        p++;
        i--;
    }
    return(ret);
}


unsigned int ndigit(unsigned int n){
    unsigned i = 1;
    
    while(n >= 10){
        n /= 10;
        i++;
    }
    return(i);
}

unsigned int upow(unsigned int x, unsigned int n){
    if(n == 0) return(1);
    if(n == 1) return(x);
    return(x * upow(x, n-1));
}


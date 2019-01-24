#include "stdlibc.h"

void *sprintf(char *s, char *format, ...){
    va_list args;
    
    char disp_digit;
    int num_flg;
    unsigned int conv_len;
    char tmp[16];
    int i;
    
    va_start(args, format);
    
    while(*format != '\0'){
        num_flg = FALSE;
        conv_len = 0;
        
        if(*format == '%'){
            format++;
            
            memset(tmp, '\0', sizeof(tmp));
            
            if(*format == '0'){
                format++;
                num_flg = TRUE;
                
                disp_digit = atoi(format);
                while(isdigit(*format)) format++;
            }
            
            if(*format == 'd'){
                conv_len = to_dec_asc(tmp, va_arg(args, int));
            }
            if(*format == 'x'){
                conv_len = to_hex_asc(tmp, va_arg(args, int), FALSE);
            }
            if(*format == 'X'){
                conv_len = to_hex_asc(tmp, va_arg(args, int), TRUE);
            }
            
            if(num_flg && (conv_len < disp_digit)){
                for(i = conv_len; i < disp_digit; i++){
                    *s = '0';
                    s++;
                }
            }
            
            memcpy(s, tmp, conv_len);
            s += conv_len;
        }
        else{
          *s = *format;
          s++;
        }
        format++;
    }
    *s = *format;
    
    va_end(args);
}

unsigned int to_dec_asc(char *buf, int n){
    char *p;
    unsigned int ret;
    unsigned int i;
    
    i = ndigit(n, 10);
    ret = i;
    p = buf;
    
    while(i > 0){
        *p = ((n / upow(10, i - 1)) % 10) + '0';
        p++;
        i--;
    }
    return(ret);
}

unsigned int to_hex_asc(char *buf, int n, int capital){
    char *p;
    unsigned int ret;
    unsigned int i;
    char charset[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    
    i = ndigit(n, 16);
    ret = i;
    p = buf;
    
    while(i > 0){
        *p = charset[((n / upow(16, i - 1)) % 16)];
        
        if(capital && !isdigit(*p)){
            *p -= 0x20;
        }
        
        p++;
        i--;
    }
    return(ret);
}


unsigned int ndigit(unsigned int n, unsigned int base){
    unsigned int i = 1;
    
    while(n >= base){
        n /= base;
        i++;
    }
    return(i);
}

unsigned int upow(unsigned int x, unsigned int n){
    if(n == 0) return(1);
    if(n == 1) return(x);
    return(x * upow(x, n-1));
}

int isdigit(char c){
    return((c >= '0' && c <= '9'));
}

int atoi(char *s){
    int result = 0;
    int sign = FALSE;
    
    //符号付き
    if(*s == '-'){
        sign = TRUE;
        s++;
    }
    
    while(isdigit(*s)){
        //すでに入ってるものを桁上げ + その桁の数値を加算
        result = (result * 10) + (*s - '0');
        s++;
    }
    
    if (sign) result *= -1;
    
    return result;
}

int iscapital(char c){
    return((c >= 'A' && c <= 'Z'));
}

char *memcpy(char *buf1, char *buf2, int n){
    int i;
    for(i = 0; i < n; i++){
            buf1[i] = buf2[i];
        }
    return buf1;
}

int memset(char *buf, char byte, int n){
    int i;
    for(i = 0; i < n; i++){
        buf[i] = byte;
    }
    return(n);
}

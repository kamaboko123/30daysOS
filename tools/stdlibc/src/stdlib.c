#include "stdlibc.h"

void *_sprintf(char *s, char *format, ...){
    va_list args;
    
    char disp_digit;
    int pad_flg;
    unsigned int conv_len;
    char tmp[16];
    int i;
    
    int sign_flg;
    int data_int;
    char pad_char;
    
    va_start(args, format);
    
    while(*format != '\0'){
        pad_flg = FALSE;
        sign_flg = FALSE;
        conv_len = 0;
        
        if(*format == '%'){
            format++;
            
            _memset(tmp, '\0', sizeof(tmp));
            
            if(*format == '0'){
                format++;
                pad_flg = TRUE;
                pad_char = '0';
                
                disp_digit = _atoi(format);
                while(_isdigit(*format)) format++;
            }
            /*
            else if(_isdigit(format)){
                pad_char = ' ';
                pad_flg = TRUE;
            }*/
            
            if(*format == 'd'){
                data_int = va_arg(args, int);
                if((data_int & 0x8000) != 0){
                    sign_flg = TRUE;
                    conv_len = _to_dec_asc(tmp, ~data_int + 1);
                }
                else{
                    conv_len = _to_dec_asc(tmp, data_int);
                }
            }
            if(*format == 'x'){
                conv_len = _to_hex_asc(tmp, va_arg(args, int), FALSE);
            }
            if(*format == 'X'){
                conv_len = _to_hex_asc(tmp, va_arg(args, int), TRUE);
            }
            
            if(sign_flg){
                *s = '-';
                s++;
            }
            
            if(pad_flg && (conv_len < disp_digit)){
                for(i = conv_len; i < disp_digit; i++){
                    *s = '0';
                    s++;
                }
            }
            
            _memcpy(s, tmp, conv_len);
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

unsigned int _to_dec_asc(char *buf, int n){
    char *p;
    unsigned int ret;
    unsigned int i;
    
    i = _ndigit(n, 10);
    ret = i;
    p = buf;
    
    while(i > 0){
        *p = ((n / _upow(10, i - 1)) % 10) + '0';
        p++;
        i--;
    }
    return(ret);
}

unsigned int _to_hex_asc(char *buf, int n, int capital){
    char *p;
    unsigned int ret;
    unsigned int i;
    char charset[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    
    i = _ndigit(n, 16);
    ret = i;
    p = buf;
    
    while(i > 0){
        *p = charset[((n / _upow(16, i - 1)) % 16)];
        
        if(capital && !_isdigit(*p)){
            *p -= 0x20;
        }
        
        p++;
        i--;
    }
    return(ret);
}


unsigned int _ndigit(unsigned int n, unsigned int base){
    unsigned int i = 1;
    
    while(n >= base){
        n /= base;
        i++;
    }
    return(i);
}

unsigned int _upow(unsigned int x, unsigned int n){
    if(n == 0) return(1);
    if(n == 1) return(x);
    return(x * _upow(x, n-1));
}

int _isdigit(char c){
    return((c >= '0' && c <= '9'));
}

int _atoi(char *s){
    int result = 0;
    int sign = FALSE;
    
    //符号付き
    if(*s == '-'){
        sign = TRUE;
        s++;
    }
    
    while(_isdigit(*s)){
        //すでに入ってるものを桁上げ + その桁の数値を加算
        result = (result * 10) + (*s - '0');
        s++;
    }
    
    if (sign) result *= -1;
    
    return result;
}

int _iscapital(char c){
    return((c >= 'A' && c <= 'Z'));
}

char *_memcpy(char *buf1, char *buf2, int n){
    int i;
    for(i = 0; i < n; i++){
            buf1[i] = buf2[i];
        }
    return buf1;
}

int _memset(char *buf, char byte, int n){
    int i;
    for(i = 0; i < n; i++){
        buf[i] = byte;
    }
    return(n);
}

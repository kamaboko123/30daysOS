#include "stdlibc.h"

unsigned int __last_rand = RAND_INIT;

unsigned int _sprintf(char *s, char *format, ...){
    va_list args;
    
    char *s_org = s;
    char disp_digit;
    int pad_flg;
    unsigned int conv_len;
    char tmp[16];
    int i;
    
    int data_int;
    char pad_char;
    
    va_start(args, format);
    
    while(*format != '\0'){
        pad_flg = FALSE;
        conv_len = 0;
        
        if(*format == '%'){
            format++;
            
            _memset(tmp, '\0', sizeof(tmp));
            
            if(*format == '\0') break;
            else if(*format == '%'){
                *s = '%';
                s++;
                goto next;
            }
            else if(_isdigit(*format)){
                pad_flg = TRUE;
                
                if(*format == '0'){
                    pad_char = '0';
                }
                else{
                    pad_char = ' ';
                }
                
                disp_digit = _atoi(format);
                while(_isdigit(*format)) format++;
            }
            
            if(*format == 'd' || *format == 'u'){
                data_int = va_arg(args, int);
                if(((data_int & 0x8000) != 0) && *format == 'd'){
                    
                    if(pad_char == ' '){
                        *tmp = '-';
                        conv_len = _to_dec_asc(tmp + 1, ~data_int + 1) + 1;
                    }
                    else if(pad_char == '0'){
                        *s = '-';
                        s++;
                        conv_len = _to_dec_asc(tmp, ~data_int + 1);
                        disp_digit--;
                    }
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
            
            if(pad_flg && (conv_len < disp_digit)){
                for(i = conv_len; i < disp_digit; i++){
                    *s = pad_char;
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
        
next:
        format++;
    }
    *s = *format;
    
    va_end(args);
    
    return ((s - s_org) / sizeof(char));
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

int _strcmp(char *s1, char *s2){
    while(*s1 != '\0' && *s2 != '\0'){
        if(*s1 != *s2) break;
        s1++;
        s2++;
    }
    
    return(*s1 - *s2);
}

int _strncmp(char *s1, char *s2, unsigned int n){
    unsigned int i = 0;
    while(*s1 != '\0' && *s2 != '\0' && ++i < n){
        if(*s1 != *s2) break;
        s1++;
        s2++;
    }
    
    return(*s1 - *s2);
}

int _strlen(char *str){
    int i;
    for(i = 0; str[i] != '\0'; i++);
    return(i);
}

int _strtol(char *s, char **endp, int base){
    int _base;
    int ret = 0;
    int sign = 0;
    
    //空白のスキップ
    while(*s == ' ') s++;
    
    if(*s == '-') sign = 1;
    
    //base = 0なら基数は渡された文字列の表記に従う
    if(base == 0){
        //渡された文字列の表記方法の検出
        //16進数
        if(_strncmp(s, "0x", 2) == 0 || _strncmp(s, "0X", 2) == 0){
            _base = 16;
        }
        //8進数
        if(_strncmp(s, "0", 1) == 0){
            _base = 8;
        }
        //それ以外は10進数
        else{
            _base = 10;
        }
    }
    else{
        _base = base;
    }
    
    //16進数
    if(_base == 16){
       while(_isdigit(*s) || (*s >= 'a' && *s <= 'f') || (*s >= 'A' && *s <= 'F') ){
            if(_isdigit(*s)){
                ret = (ret * 16) + (*s - '0');
            }
            else if(*s >= 'a' && *s <= 'f'){
                ret = (ret * 16) + (*s - 'a' + 10);
            }
            else if(*s >= 'A' && *s <= 'F'){
                ret = (ret * 16) + (*s - 'A' + 10);
            }
            s++;
        }
    }
    
    //8進数（多分使わないので省略）
    
    //10進数
    if(_base == 10){
        while(_isdigit(*s)){
            ret = (ret * 10) + (*s - '0');
            s++;
        }
    }
    
    if(sign != 0) ret *= -1;
    *endp = s;
    return ret;
}

void _rand_seed(unsigned int x){
    __last_rand = x;
}

unsigned int _rand(){
    //線形合同法による乱数生成
    //ポケモン3,4世代と同じ値
    static unsigned int a = 0x41c64e6d;
    static unsigned int b = 0x6073;
    __last_rand = (a * __last_rand + b) % (_UINT_MAX / 2);
    return __last_rand;
}


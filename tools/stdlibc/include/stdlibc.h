#ifndef STDLIBC_H
#define STDLIBC_H

#include <stdarg.h>

#define TRUE 1
#define FALSE 0

//stdlib.c
unsigned int _sprintf(char *s, char *format, ...);
unsigned int _to_dec_asc(char *buf, int n);
unsigned int _to_hex_asc(char *buf, int n, int capital);
unsigned int _ndigit(unsigned int n, unsigned int base);
unsigned int _upow(unsigned int x, unsigned int n);
int _iscapital(char c);
int _atoi(char *s);
int _isdigit(char c);
char *_memcpy(char *buf1, char *buf2, int n);
int _memset(char *buf, char byte, int n);

#endif

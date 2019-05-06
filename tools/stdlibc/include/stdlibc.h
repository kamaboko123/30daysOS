#ifndef STDLIBC_H
#define STDLIBC_H

#include <stdarg.h>

#define TRUE 1
#define FALSE 0

#define RAND_INIT 99
#define _UINT_MAX 4294967295

//stdlib.c
extern unsigned int __last_rand;
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
int _strcmp(char *s1, char *s2);
int _strncmp(char *s1, char *s2, unsigned int n);
int _strlen(char *str);

void _rand_seed(unsigned int x);
unsigned int _rand();

#endif

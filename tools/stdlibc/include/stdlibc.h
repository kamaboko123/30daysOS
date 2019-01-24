#ifndef STDLIBC_H
#define STDLIBC_H

#include <stdarg.h>

#define TRUE 1
#define FALSE 0

//stdlib.c
void *sprintf(char *s, char *format, ...);
unsigned int to_dec_asc(char *buf, int n);
unsigned int to_hex_asc(char *buf, int n, int capital);
unsigned int ndigit(unsigned int n, unsigned int base);
unsigned int upow(unsigned int x, unsigned int n);
void upcase(char *str, unsigned int n);
int iscapital(char c);
int atoi(char *s);
int isdigit(char c);
char *memcpy(char *buf1, char *buf2, int n);
int memset(char *buf, char byte, int n);

#endif

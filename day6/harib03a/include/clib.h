#ifndef CLIB_H
#define CLIB_H

void *sprintf(char *s, char *format, ...);
unsigned int to_dec_asc(char *buf, int n);
unsigned int to_hex_asc(char *buf, int n);
unsigned int ndigit(unsigned int n);
unsigned int upow(unsigned int x, unsigned int n);

#endif

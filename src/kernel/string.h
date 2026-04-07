
#ifndef STRING_H
#define STRING_H

#include <stdint.h>
#include <stddef.h>

void memset(void *p, int v, long l);
int strncmp(const char *const Lhs, const char *const Rhs, unsigned long Len);
int strnlen(const char *const A, unsigned long Len);
void memcpy(void *Destination, const void *const Source, const unsigned long Len);
char *UlToString(unsigned long Number);

#endif

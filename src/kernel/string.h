
#ifndef STRING_H
#define STRING_H

#include <stdint.h>
#include <stddef.h>

void memset(void *p, int v, long l);
int unsafe_strncmp(const char *const Lhs, const char *const Rhs, unsigned long Len);
int strnlen(char *const A, unsigned long Len);

#endif

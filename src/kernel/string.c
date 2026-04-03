
#include <string.h>
#include <panic.h>

void memset(void *p, int v, long l)
{
        PanicIfNull(p);
        unsigned char *base = (unsigned char *)p;
        for (int i = 0; i < l; ++i)
        {
                base[i] = v;
        }
}

int strncmp(const char *const Lhs, const char *const Rhs, unsigned long Len)
{
        PanicIfNull(Lhs);
        PanicIfNull(Rhs);
        for (unsigned long i = 0; i < Len; i++)
        {
                if (Lhs[i] != Rhs[i] || Lhs[i] == '\0')
                {
                        return Lhs[i] - Rhs[i];
                }
        }
        return 0;
}

int strnlen(char *const A, unsigned long Len)
{
        PanicIfNull(A);
        for (unsigned long i = 0; i < Len; ++i)
        {
                if (A[i])
                        continue;
                return i;
        }

        return Len;
}

void memcpy(void *Destination, const void *const Source, const unsigned long Len)
{
        PanicIfNull(Destination);
        PanicIfNull(Source);
        for (unsigned long i = 0; i < Len; ++i)
        {
                ((char *)Destination)[i] = ((char *)Source)[i];
        }
}

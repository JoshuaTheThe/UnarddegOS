
#include <string.h>
#include <panic.h>
#include <vmem/alloc.h>

char *UlToString(unsigned long Number)
{
        // each byte requires at most three digits
        char *const String = kalloc(sizeof(Number) * 8);
        unsigned long i;
        for (i = 0; i < sizeof(Number) * 3; ++i)
        {
                String[i] = (Number % 10) + '0';
                Number /= 10;
                if (Number == 0) break;
        }

        for (unsigned long j = 0; j < i; ++j)
        {
                char Temporary = String[j];
                String[j] = String[i - j];
                String[i - j] = Temporary;
        }

        return String;
}

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

int strnlen(const char *const A, unsigned long Len)
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

void strncpy(void *Destination, const void *const Source, const unsigned long Len)
{
        char       *Dst = (char *)Destination;
        const char *Src = (const char *)Source;
        unsigned long i = 0;
        while (i < Len && Src[i] != 0)
        {
                Dst[i] = Src[i];
                ++i;
        }
        while (i < Len)
        {
                Dst[i] = 0;
                ++i;
        }
}

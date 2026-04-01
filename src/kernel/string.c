
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

int unsafe_strncmp(const char *const Lhs, const char *const Rhs, unsigned long Len)
{
        for (unsigned long i = 0; i < Len; i++)
        {
                if (Lhs[i] != Rhs[i] || Lhs[i] == '\0')
                {
                        return Lhs[i] - Rhs[i];
                }
        }
        return 0;
}

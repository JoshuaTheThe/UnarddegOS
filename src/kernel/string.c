
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

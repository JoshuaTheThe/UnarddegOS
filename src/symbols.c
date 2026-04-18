// Auto-generated kernel symbol table
// Generated on: Sat Apr 18 22:56:08 BST 2026

#include <module.h>
#include <string.h>

struct KernelSymbol
{
        const char *name;
        void *addr;
};

static struct KernelSymbol KernelSymbols[] = {
};

const int KernelSymbolCount = sizeof(KernelSymbols) / sizeof(KernelSymbols[0]);

void *FindKernelSymbol(const char *name)
{
        for (int i = 0; i < KernelSymbolCount; i++)
        {
                if (strncmp(KernelSymbols[i].name, name, 128) == 0)
                {
                        return KernelSymbols[i].addr;
                }
        }
        return NULL;
}

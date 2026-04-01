
#include <arch.h>

// Stub
void ArchInitialise(void) {}

char *ArchIdentify(void)
{
        static char *Architecture = " [Info] ISA: Intel Architecture / 32,x86, CPU: i386\n";
        return Architecture;
}

void ArchCli(void)
{
        __asm volatile ("cli");
}

void ArchSti(void)
{
        __asm volatile ("sti");
}

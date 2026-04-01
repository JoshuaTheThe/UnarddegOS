
#include <arch.h>

// Stub
void ArchInitialise(void) {}

const char *ArchIdentify(void)
{
        static const char *Architecture = " [Info] ISA: Intel Architecture / 32,x86, CPU: i386";
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

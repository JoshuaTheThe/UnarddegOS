
#include <arch.h>

// Stub
void ArchInitialise(void) {}

char *ArchIdentify(void)
{
        static char *Architecture = " [Info] ISA: Arm32,Arm/v7, CPU: cortex-a15\n";
        return Architecture;
}

void ArchCli(void)
{
        __asm volatile ("cpsid i");
}

void ArchSti(void)
{
        __asm volatile ("cpsie i");
}

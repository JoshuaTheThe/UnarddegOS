
#include <arch.h>

// Stub
void ArchInitialise(void) {}

const char *ArchIdentify(void)
{
        static const char *Architecture = "ISA: Arm32,Arm/v7\r\nCPU: cortex-a15";
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

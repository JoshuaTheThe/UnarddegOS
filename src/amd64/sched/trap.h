#ifndef TRAP_H
#define TRAP_H

#include <io.h>

static void EnableNextProcess(void)
{
        outb(0x20, 0x20);
}

#endif

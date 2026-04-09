#ifndef TASK_H
#define TASK_H

#include <stdint.h>

typedef struct
{
        uint32_t Registers[16]; // r0..r15
        uint32_t cpsr;
} TaskRegisters;

#endif

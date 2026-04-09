#ifndef TASK_H
#define TASK_H

#include <stdint.h>

typedef struct
{
        uint64_t Registers[64]; // pc,x1.x31,f0.f31
} TaskRegisters;

#endif

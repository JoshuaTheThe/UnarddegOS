#ifndef TASK_H
#define TASK_H

#include <stdint.h>

typedef struct
{
        uint32_t Registers[16]; // r0..r15
} TaskRegisters;

#endif

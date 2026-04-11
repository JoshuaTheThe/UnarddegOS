#ifndef TASK_H
#define TASK_H

#include <stdint.h>

typedef struct
{
        uint64_t Registers[66]; // pc,x1.x31,f0.f31,mstatus,fcsr
} TaskRegisters;

#endif

#ifndef TASK_H
#define TASK_H

#include <stdint.h>

typedef struct
{
        uint64_t Registers[24]; // RAX,RCX,RDX,RBX,RSP,RBP,RSI,RDI,RIP,RCS,RDS,RES,RSS,RFS,RGS,RFLAGS,R8,R9,R10,R11,R12,R13,R14,R15
} TaskRegisters;

#endif

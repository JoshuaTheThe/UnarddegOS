#ifndef TASK_H
#define TASK_H

#include <stdint.h>

typedef struct
{
        uint32_t Registers[16]; // EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI,EIP,CS,DS,ES,SS,FS,GS,EFLAGS
} TaskRegisters;

#endif

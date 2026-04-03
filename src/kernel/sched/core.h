
#ifndef SCHED_CORE_H
#define SCHED_CORE_H

#include <sched/task.h>
#include <vfs/vnode.h>
#include <stdint.h>

// stored in file
typedef struct
{
        TaskRegisters Registers;
        uint64_t      ProgramIdentifier;
} Task;

void SchedulerInitialise(void);

#endif

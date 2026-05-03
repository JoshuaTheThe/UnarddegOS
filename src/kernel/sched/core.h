
#ifndef SCHED_CORE_H
#define SCHED_CORE_H

#include <sched/task.h>
#include <vfs/vfd.h>
#include <stdint.h>

// stored in file
typedef struct
{
        TaskRegisters   Registers;
        uint64_t        ProgramIdentifier;
        _FileDescriptor Files;
} Task;

void SchedulerInitialise(void);
VNode *SchedulerCreateProc(TaskRegisters InitialState);

extern VNode *CurrentProc;
extern Task ScratchProc;

#endif

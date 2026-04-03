
#include <sched/core.h>
#include <vmem/bumpalloc.h>
#include <panic.h>
#include <string.h>

static const char *ProcDir = "proc";

static int TaskWriteFunction(void *const Buf,
                             const unsigned long Size,
                             const unsigned long Elements,
                             VNode *const Self)
{
        const unsigned long Bytes = Size * Elements;
        if (Bytes > sizeof(Task))
                Panic(PANIC_SEGMENTATION_FAULT);
        else
                memcpy(Buf, Self->DriverData, Bytes);
        return Bytes;
}

static int TaskReadFunction(void *const Buf,
                            const unsigned long Size,
                            const unsigned long Elements,
                            VNode *const Self)
{
        const unsigned long Bytes = Size * Elements;
        if (Bytes > sizeof(Task))
                Panic(PANIC_SEGMENTATION_FAULT);
        else
                memcpy(Buf, Self->DriverData, Bytes);
        return Bytes;
}

static void SchedulerCreateProcDir(void)
{
        VNode *Proc = NewVNode(VFS_SYSTEM | VFS_READ | VFS_WRITE);
        Proc->Name.Name = ProcDir;
        Proc->Name.Length = sizeof(ProcDir) - 1;
        RegisterChildVNode(RootVNode(), Proc);
}

static void SchedulerCreateProc(TaskRegisters InitialState)
{
        static uint64_t ProgramIdentifier = 0;
        VNode *Proc     = RootVNode()->RelativeFind(RootVNode(), ProcDir, sizeof(ProcDir) - 1);
        VNode *New      = NewVNode(VFS_SYSTEM | VFS_READ | VFS_WRITE);
        New->DriverData = BumpAllocate(sizeof(Task));
        New->WriteFunction = TaskWriteFunction;
        New->ReadFunction  = TaskReadFunction;
        ((Task *)New->DriverData)->Registers = InitialState;
        ((Task *)New->DriverData)->ProgramIdentifier = ProgramIdentifier++;
        RegisterChildVNode(Proc, New);
}

void SchedulerInitialise(void)
{
        TaskRegisters InitialState = {0};
        SchedulerCreateProcDir();
        SchedulerCreateProc(InitialState);
}

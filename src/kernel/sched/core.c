
#include <sched/core.h>
#include <vmem/bumpalloc.h>
#include <panic.h>
#include <string.h>
#include <sched/trap.h>

VNode *CurrentProc = NULL, *Proc = NULL;
Task ScratchProc = {0};
static const char ProcDir[] = "proc";

void CommitProcessSave(void)
{
        CurrentProc->WriteFunction(&ScratchProc, 1, sizeof(Task), CurrentProc);
}

void CommitProcessLoad(void)
{
        CurrentProc->ReadFunction(&ScratchProc, 1, sizeof(Task), CurrentProc);
}

void CommitNextProcess(void)
{
        CurrentProc = CurrentProc->Next;
        if (CurrentProc == NULL)
        {
                CurrentProc = Proc->FirstChild;
        }
        EnableNextProcess();
        CommitProcessLoad();
}

static int TaskWriteFunction(void *const Buf,
                             const unsigned long Size,
                             const unsigned long Elements,
                             VNode *const Self)
{
        const unsigned long Bytes = Size * Elements;
        if (Bytes > sizeof(Task))
                Panic(PANIC_SEGMENTATION_FAULT);
        else
                memcpy(Self->DriverData, Buf, Bytes);
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

static VNode *SchedulerCreateProc(TaskRegisters InitialState)
{
        static uint64_t ProgramIdentifier = 0;
        Proc            = RootVNode()->RelativeFind(RootVNode(), ProcDir, sizeof(ProcDir) - 1);
        VNode *New      = NewVNode(VFS_SYSTEM | VFS_READ | VFS_WRITE);
        New->DriverData = BumpAllocate(sizeof(Task));
        New->WriteFunction = TaskWriteFunction;
        New->ReadFunction  = TaskReadFunction;
        New->Name.Name     = UlToString(ProgramIdentifier);
        New->Name.Length   = strnlen(New->Name.Name, 32);
        ((Task *)New->DriverData)->Registers = InitialState;
        ((Task *)New->DriverData)->ProgramIdentifier = ProgramIdentifier++;
        RegisterChildVNode(Proc, New);
        return New;
}

void SchedulerInitialise(void)
{
        TaskRegisters InitialState;
        memset(&InitialState, 0, sizeof(InitialState));
        SchedulerCreateProcDir();
        CurrentProc = SchedulerCreateProc(InitialState);
        EnableNextProcess();
}

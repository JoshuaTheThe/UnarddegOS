
#include <sched/core.h>
#include <vmem/alloc.h>
#include <panic.h>
#include <string.h>
#include <arch.h>
#define __NEXT_PROC
#include <sched/trap.h>

VNode *CurrentProc = NULL, *Proc = NULL, *SelfProc = NULL;
Task ScratchProc = {0};
static const char ProcDir[] = "proc";
uint64_t Ticks = 0;

void CommitProcessSave(void)
{
        if (CurrentProc)
                CurrentProc->WriteFunction(&ScratchProc, 1, sizeof(TaskRegisters), CurrentProc);
}

void CommitProcessLoad(void)
{
        if (CurrentProc)
                CurrentProc->ReadFunction(&ScratchProc, 1, sizeof(TaskRegisters), CurrentProc);
}

void CommitNextProcess(void)
{
        Ticks += 1;
        do
        {
                if (CurrentProc == NULL || CurrentProc->Next == NULL)
                {
                        CurrentProc = Proc->FirstChild;
                }
                else
                {
                        CurrentProc = CurrentProc->Next;
                }
        }
        while (CurrentProc == SelfProc);
        SelfProc->Link = CurrentProc;
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
        VNode *Self = NewVNode(VFS_SYSTEM);
        Self->Name.Name = "self";
        Self->Name.Length = 4;
        SelfProc = Self;
        RegisterChildVNode(Proc, Self);
}

VNode *SchedulerCreateProc(TaskRegisters InitialState)
{
        static uint64_t ProgramIdentifier = 0;
        Proc               = RootVNode()->RelativeFind(RootVNode(), ProcDir, sizeof(ProcDir) - 1);
        VNode *New         = NewVNode(VFS_SYSTEM | VFS_READ | VFS_WRITE);
        New->DriverData    = kalloc(sizeof(Task));
        New->WriteFunction = TaskWriteFunction;
        New->ReadFunction  = TaskReadFunction;
        New->Name.Name     = UlToString(ProgramIdentifier);
        New->Name.Length   = strnlen(New->Name.Name, 32);
        memcpy(&((Task *)New->DriverData)->Registers, &InitialState, sizeof(InitialState));
        ((Task *)New->DriverData)->ProgramIdentifier = ProgramIdentifier++;
        ((Task *)New->DriverData)->Files.FileIndex   = -1;
        ((Task *)New->DriverData)->Files.Next        = NULL;
        ((Task *)New->DriverData)->Files.Reference   = NULL;
        ((Task *)New->DriverData)->BaseDir           = RootVNode();
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

void SchedulerExit(void)
{
        ArchCli();
        VNode *Prev = Proc->FirstChild;
        if (Prev == CurrentProc)
        {
                Proc->FirstChild = CurrentProc->Next;
        }
        else
        {
                while (Prev != NULL && Prev->Next != CurrentProc)
                        Prev = Prev->Next;
                if (Prev != NULL)
                        Prev->Next = CurrentProc->Next;
        }

        Task *Tsk = CurrentProc->DriverData;
        _FileDescriptor *Desc = Tsk->Files.Next;
        while (Desc)
        {
                close(Desc->FileIndex);
                Desc=Desc->Next;
        }
        kfree(CurrentProc->DriverData);
        UnregisterVNode(CurrentProc);
        DeleteVNode(CurrentProc);
        CurrentProc = NULL;
        ArchSti();
        while(1);
}

// Auto-generated kernel symbol table
// Generated on: Sat Apr 18 22:47:32 BST 2026

#include <module.h>
#include <string.h>

struct KernelSymbol
{
        const char *name;
        void *addr;
};

static struct KernelSymbol KernelSymbols[] = {
        {"ArchCli", (void*)0x00203680},
        {"ArchIdentify", (void*)0x00203660},
        {"ArchInitialise", (void*)0x00203540},
        {"ArchSti", (void*)0x00203690},
        {"BumpAllocate", (void*)0x002033a0},
        {"CommitNextProcess", (void*)0x00201510},
        {"CommitProcessLoad", (void*)0x002014c0},
        {"CommitProcessSave", (void*)0x00201470},
        {"CreateSerialDevice", (void*)0x00201d40},
        {"CurrentProc", (void*)0x00245d28},
        {"DeleteNode", (void*)0x00203090},
        {"ExitTraceImpl", (void*)0x00201410},
        {"Files", (void*)0x00245d80},
        {"FindKernelSymbol", (void*)0x00203f50},
        {"FunctionTrace", (void*)0x00245a20},
        {"FunctionTraceDepth", (void*)0x00245d20},
        {"GdtInit", (void*)0x00203b80},
        {"GdtSetEntry", (void*)0x002038f0},
        {"GdtSetTssEntry", (void*)0x002039a0},
        {"IdtDefault", (void*)0x0020010e},
        {"IdtInit", (void*)0x00203d80},
        {"IdtSetEntry", (void*)0x00203d10},
        {"IdtTimer", (void*)0x00200108},
        {"KernelSymbolCount", (void*)0x00204040},
        {"LoadModule", (void*)0x00200720},
        {"LoadModules", (void*)0x002035c0},
        {"NewVNode", (void*)0x00203040},
        {"NextProcess", (void*)0x0020003c},
        {"PanicImpl", (void*)0x002012c0},
        {"Proc", (void*)0x00245d2c},
        {"RegisterChildVNode", (void*)0x002030f0},
        {"RegisterSiblingVNode", (void*)0x002031c0},
        {"Root", (void*)0x00245d88},
        {"RootVNode", (void*)0x00203290},
        {"SchedulerInitialise", (void*)0x002015c0},
        {"ScratchProc", (void*)0x00245d30},
        {"SerialCanRead", (void*)0x00203830},
        {"SerialInit", (void*)0x002036a0},
        {"SerialPrint", (void*)0x002001f0},
        {"SerialPrintHex", (void*)0x00200110},
        {"SerialPut", (void*)0x00203870},
        {"SerialRead", (void*)0x002038b0},
        {"SerialWaitForInput", (void*)0x002037f0},
        {"SerialWaitForTransmit", (void*)0x00203780},
        {"TimerInit", (void*)0x002034a0},
        {"TraceImpl", (void*)0x00201210},
        {"UlToString", (void*)0x00201910},
        {"VFSCreateDevices", (void*)0x002021b0},
        {"VNListTree", (void*)0x002032b0},
        {"VNodeDefault", (void*)0x00202700},
        {"current_index", (void*)0x00245e48},
        {"gdtSetTssEntry", (void*)0x00203a60},
        {"gdtTssInit", (void*)0x00203b20},
        {"kmain", (void*)0x002005d0},
        {"memcpy", (void*)0x00201c60},
        {"memory_pool", (void*)0x00245e4c},
        {"memset", (void*)0x002019f0},
        {"open", (void*)0x00202340},
        {"read", (void*)0x00202660},
        {"strncmp", (void*)0x00201a90},
        {"strnlen", (void*)0x00201bb0},
        {"write", (void*)0x00202540},
};

const int KernelSymbolCount = sizeof(KernelSymbols) / sizeof(KernelSymbols[0]);

void *FindKernelSymbol(const char *name)
{
        for (int i = 0; i < KernelSymbolCount; i++)
        {
                if (strncmp(KernelSymbols[i].name, name, 128) == 0)
                {
                        return KernelSymbols[i].addr;
                }
        }
        return NULL;
}

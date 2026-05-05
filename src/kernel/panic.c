
#include <panic.h>
#include <drivers/serial.h>
#include <arch.h>
#include <string.h>

TraceEntry FunctionTrace[MAX_TRACE_DEPTH] = {0};
int        FunctionTraceDepth = 0;

void TraceImpl(const char *const File, const char *const Func, long Line)
{
        if (FunctionTraceDepth >= MAX_TRACE_DEPTH)
                Panic(PANIC_TRACE_OVERFLOW);
        FunctionTrace[FunctionTraceDepth].FileName     = File;
        FunctionTrace[FunctionTraceDepth].FunctionName = Func;
        FunctionTrace[FunctionTraceDepth].Line         = Line;
        ++FunctionTraceDepth;
}

void ExitTraceImpl(const char *const File, const char *const Func, long Line)
{
        (void)File;
        (void)Func;
        (void)Line;
        --FunctionTraceDepth;
        memset(&FunctionTrace[FunctionTraceDepth], 0, sizeof(TraceEntry));
}

_Noreturn void PanicImpl(const char *const File, long Line, PanicCode Code, const char *const CodeAsStr, PanicClass Class)
{
        ArchCli(); // disable interrupts and say, context switching timers for given architecture
        SerialPrint("\r\n -- KERNEL PANIC VIA %s -- \r\n", Class == PANIC_CLASS_SUPERVISOR ? "SUPERVISOR" : "USERSPACE");
        SerialPrint("Kernel Panic Function %x (%s) was raised\r\n", Code, CodeAsStr);
        #ifdef HAS_TEMPERATURE
        SerialPrint("Temperature x1000: %d\r\n", ArchGetTemperatureMC());
        #endif
        if (File)
                SerialPrint("Source Location: %s:%d\r\n", File, Line);
        SerialPrint("Trace Dump:\r\n");
        if (FunctionTraceDepth == 0)
                SerialPrint("(No Trace Entries.)\r\n");
        
        for (int Depth = FunctionTraceDepth; Depth > 0; --Depth)
        {
                SerialPrint("Top-%x: %s:%d (%s)",
                            Depth,
                            FunctionTrace[Depth - 1].FileName,
                            FunctionTrace[Depth - 1].Line,
                            FunctionTrace[Depth - 1].FunctionName);
        }
        while (true)
                ArchPause();
}

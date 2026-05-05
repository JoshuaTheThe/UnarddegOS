
#ifndef PANIC_H
#define PANIC_H

#include <stddef.h>
#include <stdint.h>
#include <drivers/serial.h>

#define Panic(Code) PanicImpl(__FILE__, __LINE__, Code, #Code, PANIC_CLASS_SUPERVISOR)
#define PanicIfNull(e) do { if ((e) == NULL) Panic(PANIC_NULL_POINTER_DEREFERENCE); } while (0)

#define DEBUG

#ifdef DEBUG
#define Trace(expr, ...) do { TraceImpl(__FILE__, __func__, __LINE__); expr; ExitTraceImpl(__FILE__,__func__,__LINE__); } while(0);
#else
#define Trace(expr) expr
#endif

#define MAX_TRACE_DEPTH (64)

typedef enum
{
        PANIC_TODO,
        PANIC_RAN_OUT_OF_MEMORY,
        PANIC_NULL_POINTER_DEREFERENCE,
        PANIC_UNINTENDED_CALL,
        PANIC_TRACE_OVERFLOW,
        PANIC_NOT_FOUND,
        PANIC_SEGMENTATION_FAULT,
        PANIC_DOUBLE_OPEN,
        PANIC_FD_NOT_FOUND,
        PANIC_INCORRECT_BOOTLOADER,
        PANIC_UNHANDLED_INTERRUPT,
        PANIC_OVERHEAT,
} PanicCode;

typedef enum
{
        PANIC_CLASS_SUPERVISOR,  // Kernel bug (no user involvement)
        PANIC_CLASS_USERSPACE,   // Kernel bug triggered by user process (e.g. bad address)
} PanicClass;

typedef struct
{
        const char *FileName;
        const char *FunctionName;
        long        Line;
} TraceEntry;

_Noreturn void PanicImpl(const char *const File, long Line, PanicCode Code, const char *const CodeAsStr, PanicClass Class);
void TraceImpl(const char *const File, const char *const Func, long Line);
void ExitTraceImpl(const char *const File, const char *const Func, long Line);

#endif

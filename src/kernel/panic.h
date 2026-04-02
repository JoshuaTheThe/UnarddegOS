
#ifndef PANIC_H
#define PANIC_H

#include <stddef.h>
#include <stdint.h>
#include <drivers/serial.h>

#define Panic(Code) PanicImpl(__FILE__, __LINE__, Code, #Code)
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
} PanicCode;

typedef struct
{
        const char *FileName;
        const char *FunctionName;
        long        Line;
} TraceEntry;

_Noreturn void PanicImpl(const char *const File, long Line, PanicCode Code, const char *const CodeAsStr);
void TraceImpl(const char *const File, const char *const Func, long Line);
void ExitTraceImpl(const char *const File, const char *const Func, long Line);

#endif

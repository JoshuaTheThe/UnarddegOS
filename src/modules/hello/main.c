#include <drivers/serial.h>

int Init(void *(*FindKernelSymbol)(const char *name))
{
        (void)FindKernelSymbol;
        SerialPrint(" [Hello] Hello, World! %s\r\n");
        return 100;
}

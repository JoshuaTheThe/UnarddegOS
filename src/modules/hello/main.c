#include <drivers/serial.h>

int Init(void)
{
        SerialPrint(" [Hello] Hello, World! %s\r\n");
        return 100;
}

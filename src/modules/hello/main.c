#include <drivers/serial.h>

int Init(void)
{
        SerialPrint(" [Hello] Hello, World!\r\n");
        return 0;
}

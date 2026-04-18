int Init(void *(*FindKernelSymbol)(const char *name))
{
        void (*SerialPrint)(const char *fmt, ...) = FindKernelSymbol("SerialPrint");
        SerialPrint(" [Hello] Hello, World!\r\n");
        return 100;
}

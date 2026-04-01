#include <serial.h>

void SerialInit(void)
{
        // Disable UART
        PL011_CR = 0x00;
        // Set baud rate to 115200
        // IBRD = 26, FBRD = 3  (assuming 24MHz clock on QEMU virt)
        PL011_IBRD = 26;
        PL011_FBRD = 3;
        // 8 bits, no parity, 1 stop bit, enable FIFOs
        PL011_LCR_H = (0x3 << 5) | (1 << 4);
        // Disable all interrupts
        PL011_IMSC = 0x00;
        // Enable UART, TX, RX
        PL011_CR = (1 << 0) | (1 << 8) | (1 << 9);
}

void SerialWaitForTransmit(void)
{
        while (PL011_FR & PL011_FR_TXFF)
                ;
}

void SerialWaitForInput(void)
{
        while (!SerialCanRead())
                ;
}

void SerialPut(char c)
{
        SerialWaitForTransmit();
        PL011_DR = c;
}

bool SerialCanRead(void)
{
        return !(PL011_FR & PL011_FR_RXFE);
}

char SerialRead(void)
{
        SerialWaitForInput();
        return (char)(PL011_DR & 0xFF);
}

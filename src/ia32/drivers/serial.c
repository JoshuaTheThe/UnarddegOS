#include <drivers/serial.h>
#include <arch.h>

#define KEYBOARD_STATUS_PORT 0x64
#define KEYBOARD_DATA_PORT   0x60

void SerialInit(void)
{
        outb(SERIAL_IER, 0x00);  // Disable all interrupts
        outb(SERIAL_LCR, 0x80);  // Set DLAB (Data Access Bit) to 1
        outb(SERIAL_PORT, 0x03);  // Set LSB of baud rate divisor (e.g., 0x03 for 115200 baud)
        outb(SERIAL_IER, 0x00);  // Disable interrupts again (no need to enable them yet)
        outb(SERIAL_LCR, 0x03);  // LCR = 0x03 (8 bits, no parity, 1 stop bit)
        outb(SERIAL_PORT + 2, 0xC7);  // FIFO Control Register: Enable FIFO, clear RX/TX
        outb(SERIAL_MCR, 0x0B);  // Enable DTR and RTS for flow control
}

void SerialWaitForTransmit(void)
{
        while ((inb(SERIAL_LSR) & 0x20) == 0)
                ;
}

void SerialWaitForInput(void)
{
        while (!SerialCanRead())
                ArchPause();
}

void SerialPut(char c)
{
        SerialWaitForTransmit();
        outb(SERIAL_PORT, c);
}

bool SerialCanRead(void)
{
        return inb(SERIAL_PORT + 5) & 0x01;
}

char SerialRead(void)
{
        SerialWaitForInput();
        return inb(SERIAL_PORT);
}

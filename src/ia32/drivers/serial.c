#include <drivers/serial.h>
#define VGA_TEXT_BUFFER (0xB8000)

int X = 0, Y = 0, W = 80, H = 25;

void Scroll(void)
{
        static char *const VGA = (char *const)VGA_TEXT_BUFFER;
        for (int y = 1; y < H; y++)
        {
                for (int x = 0; x < W; x++)
                {
                        int from    = (y * W + x) * 2;
                        int to      = ((y - 1) * W + x) * 2;
                        VGA[to]     = VGA[from];
                        VGA[to + 1] = VGA[from + 1];
                }
        }
        for (int x = 0; x < W; x++)
        {
                int i      = ((H - 1) * W + x) * 2;
                VGA[i]     = ' ';
                VGA[i + 1] = 0x1e;
        }
        Y = H - 1;
}

void VGAPutCharacter(char Chr)
{
        static char *const VGA = (char *const)VGA_TEXT_BUFFER;
        if (Chr == '\r')
        {
                X = 0;
                return;
        }
        if (Chr == '\n')
        {
                Y += 1;
                X = 0;
                if (Y >= H)
                        Scroll();
                return;
        }
        VGA[(Y * W + X) * 2]     = Chr;
        VGA[(Y * W + X) * 2 + 1] = 0x1e;
        X += 1;
        if (X >= W)
        {
                X = 0;
                Y += 1;
                if (Y >= H)
                Scroll();
        }
}

void SerialInit(void)
{
        static uint16_t *const VGA = (uint16_t *const)VGA_TEXT_BUFFER;
        outb(SERIAL_IER, 0x00);  // Disable all interrupts
        outb(SERIAL_LCR, 0x80);  // Set DLAB (Data Access Bit) to 1
        outb(SERIAL_PORT, 0x03);  // Set LSB of baud rate divisor (e.g., 0x03 for 115200 baud)
        outb(SERIAL_IER, 0x00);  // Disable interrupts again (no need to enable them yet)
        outb(SERIAL_LCR, 0x03);  // LCR = 0x03 (8 bits, no parity, 1 stop bit)
        outb(SERIAL_PORT + 2, 0xC7);  // FIFO Control Register: Enable FIFO, clear RX/TX
        outb(SERIAL_MCR, 0x0B);  // Enable DTR and RTS for flow control
        for (int i = 0; i < 80 * 25; ++i)
                VGA[i] = 0x1E00;
}

void SerialWaitForTransmit(void)
{
        while ((inb(SERIAL_LSR) & 0x20) == 0)
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
        outb(SERIAL_PORT, c);
        VGAPutCharacter(c);
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

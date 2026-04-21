#include <drivers/serial.h>
#include <arch.h>
#define VGA_TEXT_BUFFER (0xB8000)
#define KEYBOARD_STATUS_PORT 0x64
#define KEYBOARD_DATA_PORT   0x60

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

const uint16_t keyboard_map[256] =
{
	0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 2, 'a', 's',
	'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '#', 'z', 'x', 'c', 'v',
	'b', 'n', 'm', ',', '.', '/', 1, '*', 1, ' ',
	2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '\a', '\v', '\r',
	'k', 'K', '-', 'h', '5', 'l', '+', 26, 'j', 'J', 25, 127, '\n',
	1, '\\', 0, 0
};

const uint16_t keyboard_map_shifted[256] =
{
	0, 27, '!', '"', '\\', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t',
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 2, 'A', 'S',
	'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '@', '`', 0, '#', 'Z', 'X', 'C', 'V',
	'B', 'N', 'M', '<', '>', '?', 1, '*', 1, ' ',
	2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '/', '7',
	'8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.', '\n',
	1, '\\', 0, 0
};

bool PS2CanRead(void)
{
	char status;
        status = inb(KEYBOARD_STATUS_PORT);
        return (status & 0x01);
}

uint16_t PS2Getch(void)
{
	char status;
	do
        {
        	status = inb(KEYBOARD_STATUS_PORT);
	} while ((status & 0x01) == 0);

	unsigned char scancode = inb(KEYBOARD_DATA_PORT);
	static int shift_pressed = 0;
	if (scancode == 0x2A)
        {
		shift_pressed = 1;
		return PS2Getch();
	}
        else if (scancode == 0xAA)
        {
		shift_pressed = 0;
		return PS2Getch();
	}
        else if (scancode & 0x80)
        {
                return PS2Getch();
        }

	return (shift_pressed ? keyboard_map_shifted[scancode] : keyboard_map[scancode]);
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

bool type=false;
void SerialWaitForInput(void)
{
        while (!SerialCanRead() && !PS2CanRead())
                ;
        type=SerialCanRead();
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
        if (type)
                return inb(SERIAL_PORT);
        else
                return PS2Getch();
}

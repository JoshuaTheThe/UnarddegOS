
#include <drivers/serial.h>
#include <stdarg.h>
#include <stddef.h>
#include <panic.h>

static void IntegerToHexadecimal(uint32_t value, char *buffer)
{
        static const char hex_digits[] = "0123456789abcdef";
        for (int i = 7; i >= 0; i--)
        {
                buffer[i] = hex_digits[value & 0xF];
                value >>= 4;
        }
        buffer[8] = '\0';
}

void SerialPrintHex(uint32_t value)
{
        char hex_buffer[9];
        IntegerToHexadecimal(value, hex_buffer);
        for (int i = 0; i < 8; i++)
        {
                SerialPut(hex_buffer[i]);
        }
}

void SerialPrint(const char *fmt, ...)
{
        va_list args;
        va_start(args, fmt);
        while (*fmt)
        {
                if (*fmt == '%')
                {
                        fmt++;
                        if (*fmt == 'c')
                        {
                                char c = (char)va_arg(args, int);
                                Trace(SerialPut(c));
                        }
                        else if (*fmt == 's')
                        {
                                char *str = va_arg(args, char *);
                                while (*str)
                                {
                                        Trace(SerialPut(*str));
                                        str++;
                                }
                        }
                        else if (*fmt == 'd')
                        {
                                int num = va_arg(args, int);
                                if (num < 0)
                                {
                                        Trace(SerialPut('-'));
                                        num = -num;
                                }
                                char buffer[20];
                                int i = 0;
                                if (num == 0)
                                {
                                        buffer[i++] = '0';
                                }
                                else
                                {
                                        while (num > 0)
                                        {
                                                buffer[i++] = (num % 10) + '0';
                                                num /= 10;
                                        }
                                }
                                for (int j = i - 1; j >= 0; j--)
                                {
                                        Trace(SerialPut(buffer[j]));
                                }
                        }
                        else if (*fmt == 'x')
                        {
                                uint32_t num = va_arg(args, uint32_t);
                                Trace(SerialPrintHex(num));
                        }
                }
                else
                {
                        Trace(SerialPut(*fmt));
                }
                fmt++;
        }

        va_end(args);
}

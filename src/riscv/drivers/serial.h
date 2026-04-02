#ifndef SERIAL_H
#define SERIAL_H

#include <common/serial.h>

// not always the same
#ifdef __ARCH_OVERRIDE_SERIAL_ADDRESS
        #define SERIAL_PORT __ARCH_OVERRIDE_SERIAL_ADDRESS
#else
        #define SERIAL_PORT 0x10000000   // Example RISC-V UART base address
#endif

#define SERIAL_IER   (SERIAL_PORT + 1)  // Interrupt Enable Register
#define SERIAL_LCR   (SERIAL_PORT + 3)  // Line Control Register
#define SERIAL_LSR   (SERIAL_PORT + 5)  // Line Status Register
#define SERIAL_MCR   (SERIAL_PORT + 4)  // Modem Control Register

#endif

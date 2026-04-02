#ifndef SERIAL_H
#define SERIAL_H

#include <common/serial.h>
#include <stdint.h>
#include <stdbool.h>

// why cant people just agree on shit
#ifdef __ARCH_OVERRIDE_SERIAL_ADDRESS
        #define PL011 __ARCH_OVERRIDE_SERIAL_ADDRESS
#else
        #define PL011_BASE  0x1c090000
#endif

#define PL011_DR    (*(volatile uint32_t *)(PL011_BASE + 0x00)) // Data Register
#define PL011_FR    (*(volatile uint32_t *)(PL011_BASE + 0x18)) // Flag Register
#define PL011_IBRD  (*(volatile uint32_t *)(PL011_BASE + 0x24)) // Integer Baud Rate
#define PL011_FBRD  (*(volatile uint32_t *)(PL011_BASE + 0x28)) // Fractional Baud Rate
#define PL011_LCR_H (*(volatile uint32_t *)(PL011_BASE + 0x2C)) // Line Control
#define PL011_CR    (*(volatile uint32_t *)(PL011_BASE + 0x30)) // Control Register
#define PL011_IMSC  (*(volatile uint32_t *)(PL011_BASE + 0x38)) // Interrupt Mask

// Flag register bits
#define PL011_FR_TXFF (1 << 5)  // TX FIFO full
#define PL011_FR_RXFE (1 << 4)  // RX FIFO empty

#endif
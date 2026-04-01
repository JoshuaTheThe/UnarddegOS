#ifndef IO_H
#define IO_H

#include <stdint.h>
#include <stdbool.h>

static inline uint8_t inb(uint32_t port)
{
        uint8_t value;
        __asm volatile("ldr %0, [%1]" : "=r"(value) : "r"(port));
        return value;
}

static inline void outb(uint32_t port, uint8_t value)
{
        __asm volatile("str %0, [%1]" : : "r"(value), "r"(port));
}

static inline uint16_t inw(uint32_t port)
{
        uint16_t value;
        __asm volatile("ldr %0, [%1]" : "=r"(value) : "r"(port));
        return value;
}

static inline void outw(uint32_t port, uint16_t value)
{
        __asm volatile("str %0, [%1]" : : "r"(value), "r"(port));
}

static inline uint32_t inl(uint32_t port)
{
        uint32_t value;
        __asm volatile("ldr %0, [%1]" : "=r"(value) : "r"(port));
        return value;
}

static inline void outl(uint32_t port, uint32_t value)
{
        __asm volatile("str %0, [%1]" : : "r"(value), "r"(port));
}

static inline void insl(uint32_t port, void *addr, uint32_t count)
{
        uint32_t *dst = (uint32_t*)addr;
        for (uint32_t i = 0; i < count; ++i)
        {
                dst[i] = inl(port);
        }
}

static inline void insw(uint32_t port, void *addr, uint32_t count)
{
        uint16_t *dst = (uint16_t*)addr;
        for (uint32_t i = 0; i < count; ++i)
        {
                dst[i] = inw(port);
        }
}

static inline void outsw(uint32_t port, const void *addr, unsigned long n)
{
        const uint16_t *src = (const uint16_t*)addr;
        for (unsigned long i = 0; i < n; ++i)
        {
                outw(port, src[i]);
        }
}

#endif
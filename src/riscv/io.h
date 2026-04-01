
#ifndef IO_H
#define IO_H

#include <stdint.h>
#include <stdbool.h>

static inline uint8_t inb(uint64_t port)
{
        uint8_t value;
        __asm volatile("lb %0, 0(%1)" : "=r"(value) : "r"(port));
        return value;
}

static inline void outb(uint64_t port, uint8_t value)
{
        __asm volatile("sb %0, 0(%1)" : : "r"(value), "r"(port));
}

static inline uint64_t inw(uint64_t port)
{
        uint64_t value;
        __asm volatile("lh %0, 0(%1)" : "=r"(value) : "r"(port));
        return value;
}

static inline void outw(uint64_t port, uint64_t value)
{
        __asm volatile("sh %0, 0(%1)" : : "r"(value), "r"(port));
}

static inline uint32_t inl(uint64_t port)
{
        uint32_t value;
        __asm volatile("lw %0, 0(%1)" : "=r"(value) : "r"(port));
        return value;
}

static inline void outl(uint64_t port, uint32_t value)
{
        __asm volatile("sw %0, 0(%1)" : : "r"(value), "r"(port));
}

static inline void insl(uint64_t port, void *addr, uint32_t count)
{
        uint32_t *dst = (uint32_t*)addr;
        for (uint32_t i = 0; i < count; ++i)
        {
                dst[i] = inl(port);
        }
}

static inline void insw(uint64_t port, void *addr, uint32_t count)
{
        uint64_t *dst = (uint64_t*)addr;
        for (uint32_t i = 0; i < count; ++i)
        {
                dst[i] = inw(port);
        }
}

static inline void outsw(uint64_t port, const void *addr, unsigned long n)
{
        const uint64_t *src = (const uint64_t*)addr;
        for (unsigned long i = 0; i < n; ++i)
        {
                outw(port, src[i]);
        }
}

#endif

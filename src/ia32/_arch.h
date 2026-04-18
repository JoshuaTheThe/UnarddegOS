#ifndef _ARCH_H
#define _ARCH_H

#include <stdint.h>

struct multiboot_tag
{
        uint32_t type;
        uint32_t size;
};

struct multiboot_info
{
        uint32_t total_size;
        uint32_t reserved;
        struct multiboot_tag tags[0];
};

#endif

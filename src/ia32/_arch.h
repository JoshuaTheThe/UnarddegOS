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

struct multiboot_tag_module
{
        uint32_t type;
        uint32_t size;
        uint32_t mod_start;
        uint32_t mod_end;
        char cmdline[];
};

#endif


#ifndef MODULE_H
#define MODULE_H

#include <stdint.h>
#include <stddef.h>

struct elf_header
{
        unsigned char magic[4];
        unsigned char bits;
        unsigned char endian;
        unsigned char version;
        unsigned char os_abi;
        unsigned char abi_version;
        unsigned char padding[7];
        unsigned short type;
        unsigned short machine;
        unsigned int version2;
        unsigned int entry;
        unsigned int phoff;
        unsigned int shoff;
        unsigned int flags;
        unsigned short ehsize;
        unsigned short phentsize;
        unsigned short phnum;
        unsigned short shentsize;
        unsigned short shnum;
        unsigned short shstrndx;
};

struct elf_section_header
{
        unsigned int sh_name;
        unsigned int sh_type;
        unsigned int sh_flags;
        unsigned int sh_addr;
        unsigned int sh_offset;
        unsigned int sh_size;
        unsigned int sh_link;
        unsigned int sh_info;
        unsigned int sh_addralign;
        unsigned int sh_entsize;
};

struct elf_symbol
{
        unsigned int st_name;
        unsigned int st_value;
        unsigned int st_size;
        unsigned char st_info;
        unsigned char st_other;
        unsigned short st_shndx;
};

struct elf_rela
{
        unsigned int r_offset;
        unsigned int r_info;
        int r_addend;
};

#define ELF32_R_TYPE(info) ((info) & 0xFF)
#define ELF32_R_SYM(info) ((info) >> 8)

#define R_386_NONE 0
#define R_386_32 1
#define R_386_PC32 2
#define R_386_GOT32 3
#define R_386_PLT32 4
#define R_386_COPY 5
#define R_386_GLOB_DAT 6
#define R_386_JUMP_SLOT 7
#define R_386_RELATIVE 8
#define R_386_GOTOFF 9
#define R_386_GOTPC 10
typedef int (*module_init_t)(void);

struct loaded_module
{
        void *base;
        size_t size;
        const char *name;
        struct elf_symbol *symtab;
        const char *strtab;
        int symcount;
};

void LoadModule(void *addr, size_t size, const char *name);

#endif

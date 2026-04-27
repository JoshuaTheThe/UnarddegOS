#include <module.h>
#include <drivers/serial.h>
#include <string.h>

static void *FindSymbol(struct loaded_module *mod, const char *name)
{
        struct elf_header *elf = (struct elf_header *)mod->base;
        struct elf_section_header *shdr =
            (struct elf_section_header *)((char *)mod->base + elf->shoff);

        for (uint32_t i = 0; i < mod->symcount; i++)
        {
                const char *sym_name = mod->strtab + mod->symtab[i].st_name;
                if (strncmp(sym_name, name, 128) == 0)
                {
                        int section_idx = mod->symtab[i].st_shndx;
                        void *result;

                        if (section_idx == SHN_ABS)
                        {
                                result = (void *)(uintptr_t)mod->symtab[i].st_value;
                        }
                        else if (section_idx == SHN_COMMON)
                        {
                                result = (void *)((char *)mod->base + mod->symtab[i].st_value);
                        }
                        else if (section_idx < elf->shnum && section_idx != SHN_UNDEF)
                        {
                                uint64_t section_offset = shdr[section_idx].sh_offset;
                                result = (void *)((char *)mod->base + section_offset + mod->symtab[i].st_value);
                        }
                        else if (section_idx == SHN_UNDEF)
                        {
                                return NULL;
                        }
                        else
                        {
                                result = (void *)((char *)mod->base + mod->symtab[i].st_value);
                        }
                        return result;
                }
        }
        return NULL;
}

static int RelocateModule(struct loaded_module *mod, struct elf_header *elf)
{
        struct elf_section_header *shdr =
            (struct elf_section_header *)((char *)mod->base + elf->shoff);
        for (int i = 0; i < elf->shnum; i++)
        {
                if (shdr[i].sh_type == SHT_SYMTAB)
                {
                        mod->symtab = (struct elf_symbol *)((char *)mod->base + shdr[i].sh_offset);
                        mod->symcount = shdr[i].sh_size / sizeof(struct elf_symbol); // ELF64_Sym = 24 bytes
                        int strtab_idx = shdr[i].sh_link;
                        mod->strtab = (const char *)mod->base + shdr[strtab_idx].sh_offset;
                        break;
                }
        }

        for (int i = 0; i < elf->shnum; i++)
        {
                if (shdr[i].sh_type != SHT_REL && shdr[i].sh_type != SHT_RELA)
                        continue;

                int target_section = shdr[i].sh_info;
                if (!(shdr[target_section].sh_flags & SHF_ALLOC))
                        continue;

                char *target_base = (char *)mod->base + shdr[target_section].sh_offset;

                if (shdr[i].sh_type == SHT_RELA)
                {
                        struct elf_rela *rel = (struct elf_rela *)((char *)mod->base + shdr[i].sh_offset);
                        int num_rel = shdr[i].sh_size / sizeof(struct elf_rela);
                        for (int r = 0; r < num_rel; r++)
                        {
                                uint32_t type = ELF64_R_TYPE(rel[r].r_info);
                                uint32_t sym_idx = ELF64_R_SYM(rel[r].r_info);
                                uintptr_t *patch_addr = (uintptr_t *)(target_base + rel[r].r_offset);
                                uintptr_t sym_addr = 0;
                                if (type == R_X86_64_RELATIVE && sym_idx == 0)
                                {
                                        *patch_addr = (uintptr_t)mod->base + rel[r].r_addend;
                                        continue;
                                }

                                if (sym_idx > 0 && sym_idx < mod->symcount)
                                {
                                        struct elf_symbol *sym = &mod->symtab[sym_idx];
                                        const char *sym_name = mod->strtab + sym->st_name;

                                        if (sym->st_shndx != SHN_UNDEF && sym->st_shndx < elf->shnum)
                                        {
                                                if (shdr[sym->st_shndx].sh_flags & SHF_ALLOC)
                                                {
                                                        uint64_t section_offset = shdr[sym->st_shndx].sh_offset;
                                                        sym_addr = (uintptr_t)mod->base + section_offset + sym->st_value;
                                                }
                                                else
                                                {
                                                        sym_addr = sym->st_value;
                                                }
                                        }
                                        else if (sym->st_shndx == SHN_UNDEF && sym_name[0] != '\0')
                                        {
                                                sym_addr = (uintptr_t)FindKernelSymbol(sym_name);
                                                if (sym_addr == 0)
                                                {
                                                        SerialPrint(" [Error] Unresolved symbol: %s\r\n", sym_name);
                                                        return -1;
                                                }
                                        }
                                        else if (sym->st_shndx == SHN_ABS)
                                        {
                                                sym_addr = sym->st_value;
                                        }
                                }

                                switch (type)
                                {
                                case R_X86_64_64:
                                        *patch_addr = sym_addr + rel[r].r_addend;
                                        break;

                                case R_X86_64_PC32:
                                case R_X86_64_PLT32:
                                {
                                        int64_t delta = (int64_t)(sym_addr + rel[r].r_addend) - (int64_t)patch_addr;
                                        *(int32_t *)patch_addr = (int32_t)delta;
                                        break;
                                }

                                case R_X86_64_GLOB_DAT:
                                case R_X86_64_JUMP_SLOT:
                                        *patch_addr = sym_addr;
                                        break;

                                case R_X86_64_RELATIVE:
                                        *patch_addr = (uintptr_t)mod->base + rel[r].r_addend;
                                        break;

                                case R_X86_64_GOTPCREL:
                                case R_X86_64_COPY:
                                default:
                                        SerialPrint(" [Warning] Unknown RELA type: %d\r\n", type);
                                        break;
                                }
                        }
                }
                else
                {
                        struct elf_rel *rel = (struct elf_rel *)((char *)mod->base + shdr[i].sh_offset);
                        int num_rel = shdr[i].sh_size / sizeof(struct elf_rel);
                        for (int r = 0; r < num_rel; r++)
                        {
                                uint32_t type = ELF64_R_TYPE(rel[r].r_info);
                                uint32_t sym_idx = ELF64_R_SYM(rel[r].r_info);
                                uintptr_t *patch_addr = (uintptr_t *)(target_base + rel[r].r_offset);
                                uintptr_t sym_addr = 0;
                                uintptr_t addend = *patch_addr;
                                if (sym_idx > 0 && sym_idx < mod->symcount)
                                {
                                        struct elf_symbol *sym = &mod->symtab[sym_idx];
                                        const char *sym_name = mod->strtab + sym->st_name;
                                        if (sym->st_shndx != SHN_UNDEF && sym->st_shndx < elf->shnum)
                                        {
                                                if (shdr[sym->st_shndx].sh_flags & SHF_ALLOC)
                                                {
                                                        uint64_t section_offset = shdr[sym->st_shndx].sh_offset;
                                                        sym_addr = (uintptr_t)mod->base + section_offset + sym->st_value;
                                                }
                                                else
                                                {
                                                        sym_addr = sym->st_value;
                                                }
                                        }
                                        else if (sym->st_shndx == SHN_UNDEF && sym_name[0] != '\0')
                                        {
                                                sym_addr = (uintptr_t)FindKernelSymbol(sym_name);
                                                if (sym_addr == 0)
                                                {
                                                        SerialPrint(" [Error] Unresolved symbol: %s\r\n", sym_name);
                                                        return -1;
                                                }
                                        }
                                }

                                switch (type)
                                {
                                case R_X86_64_64:
                                        *patch_addr = sym_addr + addend;
                                        break;

                                case R_X86_64_PC32:
                                case R_X86_64_PLT32:
                                {
                                        int64_t delta = (int64_t)(sym_addr + addend) - (int64_t)patch_addr;
                                        *(int32_t *)patch_addr = (int32_t)delta;
                                        break;
                                }

                                case R_X86_64_GLOB_DAT:
                                case R_X86_64_JUMP_SLOT:
                                        *patch_addr = sym_addr;
                                        break;

                                default:
                                        SerialPrint(" [Warning] Unknown REL type: %d\r\n", type);
                                        break;
                                }
                        }
                }
        }
        return 0;
}

static void CallConstructors(struct loaded_module *mod)
{
        struct elf_header *elf = (struct elf_header *)mod->base;
        struct elf_section_header *shdr = (struct elf_section_header *)((char *)mod->base + elf->shoff);

        for (int i = 0; i < elf->shnum; i++)
        {
                if (shdr[i].sh_type == SHT_INIT_ARRAY)
                {
                        if (!(shdr[i].sh_flags & SHF_ALLOC))
                                continue;

                        void (**init_funcs)(void) = (void (**)(void))((char *)mod->base + shdr[i].sh_offset);
                        int count = shdr[i].sh_size / sizeof(void *);

                        for (int c = 0; c < count; c++)
                        {
                                if (init_funcs[c])
                                {
                                        SerialPrint(" [Info] Calling constructor %d at 0x%x\r\n", c, init_funcs[c]);
                                        init_funcs[c]();
                                }
                        }
                }
        }
}

void LoadModule(void *addr, size_t size, const char *name)
{
        SerialPrint(" [Info] Loading module: %s (%d bytes)\r\n", name, size);

        unsigned char *magic = (unsigned char *)addr;
        if (magic[0] == 0x7F && magic[1] == 'E' &&
            magic[2] == 'L' && magic[3] == 'F')
        {
                struct elf_header *elf = (struct elf_header *)addr;
                if (elf->type != ET_REL)
                {
                        SerialPrint(" [Error] Not relocatable ELF (type=%d)\r\n", elf->type);
                        return;
                }

                struct loaded_module mod;
                mod.base = addr;
                mod.size = size;
                mod.name = name;
                mod.symtab = NULL;
                mod.strtab = NULL;
                mod.symcount = 0;

                if (RelocateModule(&mod, elf) != 0)
                {
                        SerialPrint(" [Error] Relocation failed\r\n");
                        return;
                }

                int (*init_func)(void *) = FindSymbol(&mod, "Init");
                int result = 0;
                if (init_func)
                {
                        SerialPrint(" [Info] Calling Init at 0x%x\r\n", init_func);
                        result = init_func(FindKernelSymbol);
                        SerialPrint(" [Info] Init returned %d\r\n", result);
                }
                else
                {
                        SerialPrint(" [Warning] No Init symbol found\r\n");
                }

                CallConstructors(&mod);
                SerialPrint(" [Info] Module loaded successfully\r\n");
        }
        else
        {
                SerialPrint(" [Error] Not an ELF file\r\n");
        }
}

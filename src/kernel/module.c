#include <module.h>
#include <drivers/serial.h>
#include <string.h>

static void *FindSymbol(struct loaded_module *mod, const char *name)
{
        struct elf_header *elf = (struct elf_header *)mod->base;
        struct elf_section_header *shdr = (struct elf_section_header *)((char *)mod->base + elf->shoff);
        
        for (int i = 0; i < mod->symcount; i++)
        {
                const char *sym_name = mod->strtab + mod->symtab[i].st_name;
                if (strncmp(sym_name, name, 128) == 0)
                {
                        int section_idx = mod->symtab[i].st_shndx;
                        void *result;
                        
                        if (section_idx < elf->shnum)
                        {
                                unsigned int section_offset = shdr[section_idx].sh_offset;
                                result = (void *)((char *)mod->base + section_offset + mod->symtab[i].st_value);
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
        struct elf_section_header *shdr = (struct elf_section_header *)((char *)mod->base + elf->shoff);
        for (int i = 0; i < elf->shnum; i++)
        {
                if (shdr[i].sh_type == 2) // SHT_SYMTAB
                {
                        mod->symtab = (struct elf_symbol *)((char *)mod->base + shdr[i].sh_offset);
                        mod->symcount = shdr[i].sh_size / sizeof(struct elf_symbol);
                        int strtab_idx = shdr[i].sh_link;
                        mod->strtab = (const char *)mod->base + shdr[strtab_idx].sh_offset;
                }
        }

        // Apply all relocations
        for (int i = 0; i < elf->shnum; i++)
        {
                if (shdr[i].sh_type == 4 || shdr[i].sh_type == 0x12) // SHT_REL or SHT_RELA
                {
                        struct elf_rela *rel = (struct elf_rela *)((char *)mod->base + shdr[i].sh_offset);
                        int num_rel = shdr[i].sh_size / sizeof(struct elf_rela);

                        // Which section this relocation applies to
                        int target_section = shdr[i].sh_info;
                        char *target_base = (char *)mod->base + shdr[target_section].sh_offset;

                        for (int r = 0; r < num_rel; r++)
                        {
                                int type = ELF32_R_TYPE(rel[r].r_info);
                                int sym_idx = ELF32_R_SYM(rel[r].r_info);

                                // Where to apply the relocation
                                uint32_t *patch_addr = (uint32_t *)(target_base + rel[r].r_offset);

                                // Symbol address (if any)
                                uint32_t sym_addr = 0;
                                if (sym_idx > 0)
                                {
                                        struct elf_symbol *sym = &mod->symtab[sym_idx];
                                        const char *sym_name = mod->strtab + sym->st_name;

                                        // Check if it's an internal symbol (in this module)
                                        if (sym->st_shndx != 0) // SHN_UNDEF = undefined
                                        {
                                                sym_addr = (uint32_t)mod->base + sym->st_value;
                                        }
                                        else
                                        {
                                                // External symbol - need kernel to provide it
                                                SerialPrint(" [Info] Module needs external symbol: %s\r\n", sym_name);
                                                // For now, just skip
                                                continue;
                                        }
                                }

                                switch (type)
                                {
                                case R_386_NONE:
                                        break;

                                case R_386_32: // Absolute: S + A
                                        *patch_addr = sym_addr + rel[r].r_addend;
                                        break;

                                case R_386_PC32: // Relative: S + A - P
                                        *patch_addr = sym_addr + rel[r].r_addend -
                                                      (uint32_t)patch_addr;
                                        break;

                                case R_386_RELATIVE: // Base + A
                                        *patch_addr = (uint32_t)mod->base + rel[r].r_addend;
                                        break;

                                case R_386_GLOB_DAT: // S
                                case R_386_JUMP_SLOT:
                                        *patch_addr = sym_addr;
                                        break;

                                default:
                                        SerialPrint(" [Warning] Unknown relocation type: %d at 0x%x\r\n",
                                                    type, (unsigned int)patch_addr);
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
                if (shdr[i].sh_type == 0x12)
                {
                        void (**init_funcs)(void) = (void (**)(void))((char *)mod->base + shdr[i].sh_offset);
                        int count = shdr[i].sh_size / sizeof(void *);

                        for (int c = 0; c < count; c++)
                        {
                                if (init_funcs[c])
                                {
                                        SerialPrint(" [Info] Calling constructor %d at 0x%x\r\n", c,
                                                    (unsigned int)init_funcs[c]);
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
                if (elf->type != 1)
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
                if (RelocateModule(&mod, elf) != 0)
                {
                        SerialPrint(" [Error] Relocation failed\r\n");
                        return;
                }

                void (*init_func)(void) = FindSymbol(&mod, "Init");
                if (init_func)
                {
                        SerialPrint(" [Info] Calling Init at 0x%x\r\n",
                                    (unsigned int)init_func);
                        init_func();
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

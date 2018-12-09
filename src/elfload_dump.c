#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>

#include "box86version.h"
#include "elfloader.h"
#include "debug.h"
#include "elfload_dump.h"
#include "elfloader_private.h"

const char* DumpSection(Elf32_Shdr *s, char* SST) {
    static char buff[200];
    switch (s->sh_type) {
        case SHT_NULL:
            return "SHT_NULL";
        #define GO(A) \
        case A:     \
            sprintf(buff, #A " Name=\"%s\"(%d) off=0x%X, size=%d, attr=0x%04X, addr=%p(%02X), link/info=%d/%d", \
                SST+s->sh_name, s->sh_name, s->sh_offset, s->sh_size, s->sh_flags, s->sh_addr, s->sh_addralign, s->sh_link, s->sh_info); \
            break
        GO(SHT_PROGBITS);
        GO(SHT_SYMTAB);
        GO(SHT_STRTAB);
        GO(SHT_RELA);
        GO(SHT_HASH);
        GO(SHT_DYNAMIC);
        GO(SHT_NOTE);
        GO(SHT_NOBITS);
        GO(SHT_REL);
        GO(SHT_SHLIB);
        GO(SHT_DYNSYM);
        GO(SHT_INIT_ARRAY);
        GO(SHT_FINI_ARRAY);
        GO(SHT_PREINIT_ARRAY);
        GO(SHT_GROUP);
        GO(SHT_SYMTAB_SHNDX);
        GO(SHT_NUM);
        GO(SHT_LOPROC);
        GO(SHT_HIPROC);
        GO(SHT_LOUSER);
        GO(SHT_HIUSER);
        GO(SHT_GNU_versym);
        GO(SHT_GNU_ATTRIBUTES);
        GO(SHT_GNU_HASH);
        GO(SHT_GNU_LIBLIST);
        GO(SHT_CHECKSUM);
        GO(SHT_LOSUNW);
        //GO(SHT_SUNW_move);
        GO(SHT_SUNW_COMDAT);
        GO(SHT_SUNW_syminfo);
        GO(SHT_GNU_verdef);
        GO(SHT_GNU_verneed);
        #undef GO
        default:
            sprintf(buff, "0x%X unknown type", s->sh_type);
    }
    return buff;
}

const char* DumpDynamic(Elf32_Dyn *s) {
    static char buff[200];
    switch (s->d_tag) {
        case DT_NULL:
            return "DT_NULL: End Dynamic Section";
        #define GO(A, Add) \
        case A:     \
            sprintf(buff, #A " %s=0x%X", (Add)?"Addr":"Val", (Add)?s->d_un.d_ptr:s->d_un.d_val); \
            break
            GO(DT_NEEDED, 0);
            GO(DT_PLTRELSZ, 0);
            GO(DT_PLTGOT, 1);
            GO(DT_HASH, 1);
            GO(DT_STRTAB, 1);
            GO(DT_SYMTAB, 1);
            GO(DT_RELA, 1);
            GO(DT_RELASZ, 0);
            GO(DT_RELAENT, 0);
            GO(DT_STRSZ, 0);
            GO(DT_SYMENT, 0);
            GO(DT_INIT, 1);
            GO(DT_FINI, 1);
            GO(DT_SONAME, 0);
            GO(DT_RPATH, 0);
            GO(DT_SYMBOLIC, 0);
            GO(DT_REL, 1);
            GO(DT_RELSZ, 0);
            GO(DT_RELENT, 0);
            GO(DT_PLTREL, 0);
            GO(DT_DEBUG, 0);
            GO(DT_TEXTREL, 0);
            GO(DT_JMPREL, 1);
            GO(DT_BIND_NOW, 1);
            GO(DT_INIT_ARRAY, 1);
            GO(DT_FINI_ARRAY, 1);
            GO(DT_INIT_ARRAYSZ, 0);
            GO(DT_FINI_ARRAYSZ, 0);
            GO(DT_RUNPATH, 0);
            GO(DT_FLAGS, 0);
            GO(DT_ENCODING, 0);
            GO(DT_NUM, 0);
            GO(DT_VALRNGLO, 0);
            GO(DT_GNU_PRELINKED, 0);
            GO(DT_GNU_CONFLICTSZ, 0);
            GO(DT_GNU_LIBLISTSZ, 0);
            GO(DT_CHECKSUM, 0);
            GO(DT_PLTPADSZ, 0);
            GO(DT_MOVEENT, 0);
            GO(DT_MOVESZ, 0);
            GO(DT_FEATURE_1, 0);
            GO(DT_POSFLAG_1, 0);
            GO(DT_SYMINSZ, 0);
            GO(DT_SYMINENT, 0);
            GO(DT_ADDRRNGLO, 0);
            GO(DT_GNU_HASH, 0);
            GO(DT_TLSDESC_PLT, 0);
            GO(DT_TLSDESC_GOT, 0);
            GO(DT_GNU_CONFLICT, 0);
            GO(DT_GNU_LIBLIST, 0);
            GO(DT_CONFIG, 0);
            GO(DT_DEPAUDIT, 0);
            GO(DT_AUDIT, 0);
            GO(DT_PLTPAD, 0);
            GO(DT_MOVETAB, 0);
            GO(DT_SYMINFO, 0);
            GO(DT_VERSYM, 0);
            GO(DT_RELACOUNT, 0);
            GO(DT_RELCOUNT, 0);
            GO(DT_FLAGS_1, 0);
            GO(DT_VERDEF, 0);
            GO(DT_VERDEFNUM, 0);
            GO(DT_VERNEED, 0);
            GO(DT_VERNEEDNUM, 0);
            GO(DT_AUXILIARY, 0);
            GO(DT_FILTER, 0);
        #undef GO
        default:
            sprintf(buff, "0x%X unknown type", s->d_tag);
    }
    return buff;
}

const char* DumpPHEntry(Elf32_Phdr *e)
{
    static char buff[500];
    memset(buff, 0, sizeof(buff));
    switch(e->p_type) {
        case PT_NULL: sprintf(buff, "type: %s", "PT_NULL"); break;
        #define GO(T) case T: sprintf(buff, "type: %s, Off=%x vaddr=%p paddr=%p filesz=%u memsz=%u flags=%x align=%u", #T, e->p_offset, e->p_vaddr, e->p_paddr, e->p_filesz, e->p_memsz, e->p_flags, e->p_align); break
        GO(PT_LOAD);
        GO(PT_DYNAMIC);
        GO(PT_INTERP);
        GO(PT_NOTE);
        GO(PT_SHLIB);
        GO(PT_PHDR);
        GO(PT_TLS);
        GO(PT_NUM);
        GO(PT_LOOS);
        GO(PT_GNU_EH_FRAME);
        GO(PT_GNU_STACK);
        GO(PT_GNU_RELRO);
        default: sprintf(buff, "type: %x, Off=%x vaddr=%p paddr=%p filesz=%u memsz=%u flags=%x align=%u", e->p_type, e->p_offset, e->p_vaddr, e->p_paddr, e->p_filesz, e->p_memsz, e->p_flags, e->p_align); break;
    }
    return buff;
}


void DumpMainHeader(Elf32_Ehdr *header, elfheader_t *h)
{
    if(box86_debug>=DEBUG_DUMP) {
        printf_debug(DEBUG_DUMP, "ELF Dump main header\n");
        printf_debug(DEBUG_DUMP, "  Entry point = %p\n", header->e_entry);
        printf_debug(DEBUG_DUMP, "  Program Header table offset = %p\n", header->e_phoff);
        printf_debug(DEBUG_DUMP, "  Section Header table offset = %p\n", header->e_shoff);
        printf_debug(DEBUG_DUMP, "  Flags = 0x%X\n", header->e_flags);
        printf_debug(DEBUG_DUMP, "  ELF Header size = %d\n", header->e_ehsize);
        printf_debug(DEBUG_DUMP, "  Program Header Entry num/size = %d(%d)/%d\n", h->numPHEntries, header->e_phnum, header->e_phentsize);
        printf_debug(DEBUG_DUMP, "  Section Header Entry num/size = %d(%d)/%d\n", h->numSHEntries, header->e_shnum, header->e_shentsize);
        printf_debug(DEBUG_DUMP, "  Section Header index num = %d(%d)\n", h->SHIdx, header->e_shstrndx);
        printf_debug(DEBUG_DUMP, "ELF Dump ==========\n");

        printf_debug(DEBUG_DUMP, "ELF Dump PEntries (%d)\n", h->numSHEntries);
        for (int i=0; i<h->numPHEntries; ++i)
            printf_debug(DEBUG_DUMP, "  PHEntry %04d : %s\n", i, DumpPHEntry(h->PHEntries+i));
        printf_debug(DEBUG_DUMP, "ELF Dump PEntries ====\n");

        printf_debug(DEBUG_DUMP, "ELF Dump Sections (%d)\n", h->numSHEntries);
        for (int i=0; i<h->numSHEntries; ++i)
            printf_debug(DEBUG_DUMP, "  Section %04d : %s\n", i, DumpSection(h->SHEntries+i, h->SHStrTab));
        printf_debug(DEBUG_DUMP, "ELF Dump Sections ====\n");
    }
}

void DumpSymTab(elfheader_t *h)
{
    if(box86_debug>=DEBUG_DUMP && h->SymTab) {
        printf_debug(DEBUG_DUMP, "ELF Dump SymTab(%d)\n", h->numSymTab);
        for (int i=0; i<h->numSymTab; ++i)
            printf_debug(DEBUG_DUMP, "  SymTab[%d] = \"%s\", value=%p, size=%d, info/other=%d/%d index=%d\n", 
                i, h->StrTab+h->SymTab[i].st_name, h->SymTab[i].st_value, h->SymTab[i].st_size,
                h->SymTab[i].st_info, h->SymTab[i].st_other, h->SymTab[i].st_shndx);
        printf_debug(DEBUG_DUMP, "ELF Dump SymTab=====\n");
    }
}

void DumpDynamicSections(elfheader_t *h)
{
    if(box86_debug>=DEBUG_DUMP && h->Dynamic) {
        printf_debug(DEBUG_DUMP, "ELF Dump Dynamic(%d)\n", h->numDynamic);
        for (int i=0; i<h->numDynamic; ++i)
            printf_debug(DEBUG_DUMP, "  Dynamic %04d : %s\n", i, DumpDynamic(h->Dynamic+i));
        printf_debug(DEBUG_DUMP, "ELF Dump Dynamic=====\n");
    }
}

void DumpDynSym(elfheader_t *h)
{
    if(box86_debug>=DEBUG_DUMP && h->DynSym) {
        printf_debug(DEBUG_DUMP, "ELF Dump DynSym(%d)\n", h->numDynSym);
        for (int i=0; i<h->numDynSym; ++i)
            printf_debug(DEBUG_DUMP, "  DynSym[%d] = \"%s\", value=%p, size=%d, info/other=%d/%d index=%d\n", 
                i, h->DynStr+h->DynSym[i].st_name, h->DynSym[i].st_value, h->DynSym[i].st_size,
                h->DynSym[i].st_info, h->DynSym[i].st_other, h->DynSym[i].st_shndx);
        printf_debug(DEBUG_DUMP, "ELF Dump DynSym=====\n");
    }
}
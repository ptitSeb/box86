#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>

#include "box86version.h"
#include "elfloader.h"
#include "debug.h"

#ifndef PN_XNUM 
#define PN_XNUM (0xffff)
#endif

typedef struct {
    int         numPHEntries;
    Elf32_Phdr  *PHEntries;
    int         numSHEntries;
    Elf32_Shdr  *SHEntries;
    int         SHIdx;
    int         numSST;
    char*       SHStrTab;
    char*       StrTab;
    Elf32_Sym*  SymTab;
    int         numSymTab;
    char*       DynStr;
    Elf32_Sym*  DynSym;
    int         numDynSym;
} elfheader_t;

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

int LoadSH(FILE *f, Elf32_Shdr *s, void** SH, const char* name, uint32_t type)
{
    if(type && (s->sh_type != type)) {
        printf_debug(DEBUG_INFO, "Section Header \"%s\" (off=%d, size=%d) has incorect type (%d != %d)\n", name, s->sh_offset, s->sh_size, s->sh_type, type);
        return -1;
    }
    if (type==SHT_SYMTAB && s->sh_size%sizeof(Elf32_Sym)) {
        printf_debug(DEBUG_INFO, "Section Header \"%s\" (off=%d, size=%d) has size (not multiple of %d)\n", name, s->sh_offset, s->sh_size, sizeof(Elf32_Sym));
    }
    *SH = calloc(1, s->sh_size);
    fseek(f, s->sh_offset ,SEEK_SET);
    if(fread(*SH, s->sh_size, 1, f)!=1) {
            printf_debug(DEBUG_INFO, "Cannot read Section Header \"%s\" (off=%d, size=%d)\n", name, s->sh_offset, s->sh_size);
            return -1;
    }

    return 0;
}

int FindSection(Elf32_Shdr *s, int n, char* SHStrTab, const char* name)
{
    for (int i=0; i<n; ++i) {
        if(s[i].sh_type!=SHT_NULL)
            if(!strcmp(SHStrTab+s[i].sh_name, name))
                return i;
    }
    return 0;
}

void LoadNamedSection(FILE *f, Elf32_Shdr *s, int size, char* SHStrTab, const char* name, const char* clearname, uint32_t type, void** what, int* num)
{
    int n = FindSection(s, size, SHStrTab, name);
    printf_debug(DEBUG_DEBUG, "Loading %s (idx = %d)\n", clearname, n);
    if(n)
        LoadSH(f, s+n, what, name, type);
    if(type==SHT_SYMTAB || type==SHT_DYNSYM) {
        if(*what && num)
            *num = s[n].sh_size / sizeof(Elf32_Sym);
    }
}

void* LoadAndCheckElfHeader(FILE* f, int exec)
{
    Elf32_Ehdr header;
    if(fread(&header, sizeof(Elf32_Ehdr), 1, f)!=1) {
        printf_debug(DEBUG_INFO, "Cannot read ELF Header\n");
        return NULL;
    }
    if(memcmp(header.e_ident, ELFMAG, SELFMAG)!=0) {
        printf_debug(DEBUG_INFO, "Not an ELF file (sign=%c%c%c%c)\n", header.e_ident[0], header.e_ident[1], header.e_ident[2], header.e_ident[3]);
        return NULL;
    }
    if(header.e_ident[EI_CLASS]!=ELFCLASS32) {
        printf_debug(DEBUG_INFO, "Not an 32bits ELF (%d)\n", header.e_ident[EI_CLASS]);
        return NULL;
    }
    if(header.e_ident[EI_DATA]!=ELFDATA2LSB) {
        printf_debug(DEBUG_INFO, "Not an LittleEndian ELF (%d)\n", header.e_ident[EI_DATA]);
        return NULL;
    }
    if(header.e_ident[EI_VERSION]!=EV_CURRENT) {
        printf_debug(DEBUG_INFO, "Incorrect ELF version (%d)\n", header.e_ident[EI_VERSION]);
        return NULL;
    }
    if(header.e_ident[EI_OSABI]!=ELFOSABI_LINUX && header.e_ident[EI_OSABI]!=ELFOSABI_NONE && header.e_ident[EI_OSABI]!=ELFOSABI_SYSV) {
        printf_debug(DEBUG_INFO, "Not a Linux ELF (%d)\n",header.e_ident[EI_OSABI]);
        return NULL;
    }

    if(exec) {
        if(header.e_type != ET_EXEC) {
            printf_debug(DEBUG_INFO, "Not an Executable (%d)\n", header.e_type);
            return NULL;
        }
    } else {
        if(header.e_type != ET_DYN) {
            printf_debug(DEBUG_INFO, "Not an Library (%d)\n", header.e_type);
            return NULL;
        }
    }

    if(header.e_machine != EM_386) {
        printf_debug(DEBUG_INFO, "Not an i386 ELF (%d)\n", header.e_machine);
        return NULL;
    }

    if(header.e_entry == 0) {
        printf_debug(DEBUG_INFO, "No entry point in ELF\n");
        return NULL;
    }
    if(header.e_phentsize != sizeof(Elf32_Phdr)) {
        printf_debug(DEBUG_INFO, "Program Header Entry size incorrect (%d != %d)\n", header.e_phentsize, sizeof(Elf32_Phdr));
        return NULL;
    }
    if(header.e_shentsize != sizeof(Elf32_Shdr)) {
        printf_debug(DEBUG_INFO, "Section Header Entry size incorrect (%d != %d)\n", header.e_shentsize, sizeof(Elf32_Shdr));
        return NULL;
    }

    elfheader_t *h = calloc(1, sizeof(elfheader_t));
    h->numPHEntries = header.e_phnum;
    h->numSHEntries = header.e_shnum;
    h->SHIdx = header.e_shstrndx;
    // special cases for nums
    if(h->numSHEntries == 0) {
        printf_debug(DEBUG_DEBUG, "Read number of Sections in 1st Section\n");
        // read 1st section header and grab actual number from here
        fseek(f, header.e_shoff, SEEK_SET);
        Elf32_Shdr section;
        if(fread(&section, sizeof(Elf32_Shdr), 1, f)!=1) {
            free(h);
            printf_debug(DEBUG_INFO, "Cannot read Initial Section Header\n");
            return NULL;
        }
        h->numSHEntries = section.sh_size;
    }
    // now read all section headers
    printf_debug(DEBUG_DEBUG, "Read %d Section header\n", h->numSHEntries);
    h->SHEntries = (Elf32_Shdr*)calloc(h->numSHEntries, sizeof(Elf32_Shdr));
    fseek(f, header.e_shoff ,SEEK_SET);
    if(fread(h->SHEntries, sizeof(Elf32_Shdr), h->numSHEntries, f)!=h->numSHEntries) {
            FreeElfHeader((void**)&h);
            printf_debug(DEBUG_INFO, "Cannot read all Section Header\n");
            return NULL;
    }

    if(h->numPHEntries == PN_XNUM) {
        printf_debug(DEBUG_DEBUG, "Read number of Program Header in 1st Section\n");
        // read 1st section header and grab actual number from here
        h->numPHEntries = h->SHEntries[0].sh_info;
    }

    printf_debug(DEBUG_DEBUG, "Read %d Program header\n", h->numPHEntries);
    h->PHEntries = (Elf32_Phdr*)calloc(h->numPHEntries, sizeof(Elf32_Phdr));
    fseek(f, header.e_phoff ,SEEK_SET);
    if(fread(h->PHEntries, sizeof(Elf32_Phdr), h->numPHEntries, f)!=h->numPHEntries) {
            FreeElfHeader((void**)&h);
            printf_debug(DEBUG_INFO, "Cannot read all Program Header\n");
            return NULL;
    }

    if(h->SHIdx == SHN_XINDEX) {
        printf_debug(DEBUG_DEBUG, "Read number of String Table in 1st Section\n");
        h->SHIdx = h->SHEntries[0].sh_link;
    }
    if(h->SHIdx > h->numSHEntries) {
        printf_debug(DEBUG_INFO, "Incoherent Section String Table Index : %d / %d\n", h->SHIdx, h->numSHEntries);
        FreeElfHeader((void**)&h);
        return NULL;
    }
    // load Section table
    printf_debug(DEBUG_DEBUG, "Loading Sections Table String (idx = %d)\n", h->SHIdx);
    if(LoadSH(f, h->SHEntries+h->SHIdx, (void*)&h->SHStrTab, ".shstrtab", SHT_STRTAB)) {
        FreeElfHeader((void**)&h);
        return NULL;
    }

    if(box86_debug>=DEBUG_DEBUG) {
        printf_debug(DEBUG_DEBUG, "ELF Dump main header\n");
        printf_debug(DEBUG_DEBUG, "  Entry point = %p\n", header.e_entry);
        printf_debug(DEBUG_DEBUG, "  Program Header table offset = %p\n", header.e_phoff);
        printf_debug(DEBUG_DEBUG, "  Section Header table offset = %p\n", header.e_shoff);
        printf_debug(DEBUG_DEBUG, "  Flags = 0x%X\n", header.e_flags);
        printf_debug(DEBUG_DEBUG, "  ELF Header size = %d\n", header.e_ehsize);
        printf_debug(DEBUG_DEBUG, "  Program Header Entry num/size = %d(%d)/%d\n", h->numPHEntries, header.e_phnum, header.e_phentsize);
        printf_debug(DEBUG_DEBUG, "  Section Header Entry num/size = %d(%d)/%d\n", h->numSHEntries, header.e_shnum, header.e_shentsize);
        printf_debug(DEBUG_DEBUG, "  Section Header index num = %d(%d)\n", h->SHIdx, header.e_shstrndx);
        printf_debug(DEBUG_DEBUG, "ELF Dump ==========\n");

        printf_debug(DEBUG_DEBUG, "ELF Dump Sections (%d)\n", h->numSHEntries);
        for (int i=0; i<h->numSHEntries; ++i)
            printf_debug(DEBUG_DEBUG, "  Section %04d : %s\n", i, DumpSection(h->SHEntries+i, h->SHStrTab));
        printf_debug(DEBUG_DEBUG, "ELF Dump Sections ====\n");
    }

    LoadNamedSection(f, h->SHEntries, h->numSHEntries, h->SHStrTab, ".strtab", "SymTab Strings", SHT_STRTAB, (void**)&h->StrTab, NULL);
    LoadNamedSection(f, h->SHEntries, h->numSHEntries, h->SHStrTab, ".symtab", "SymTab", SHT_SYMTAB, (void**)&h->SymTab, &h->numSymTab);

    if(box86_debug>=DEBUG_DEBUG && h->SymTab) {
        printf_debug(DEBUG_DEBUG, "ELF Dump SymTab(%d)\n", h->numSymTab);
        for (int i=0; i<h->numSymTab; ++i)
            printf_debug(DEBUG_DEBUG, "  SymTab[%d] = \"%s\", value=%p, size=%d, info/other=%d/%d index=%d\n", 
                i, h->StrTab+h->SymTab[i].st_name, h->SymTab[i].st_value, h->SymTab[i].st_size,
                h->SymTab[i].st_info, h->SymTab[i].st_other, h->SymTab[i].st_shndx);
        printf_debug(DEBUG_DEBUG, "ELF Dump SymTab=====\n", h->numSymTab);
    }

    LoadNamedSection(f, h->SHEntries, h->numSHEntries, h->SHStrTab, ".dynstr", "DynSym Strings", SHT_STRTAB, (void**)&h->DynStr, NULL);
    LoadNamedSection(f, h->SHEntries, h->numSHEntries, h->SHStrTab, ".dynsym", "DynSym", SHT_DYNSYM, (void**)&h->DynSym, &h->numDynSym);

    if(box86_debug>=DEBUG_DEBUG && h->DynSym) {
        printf_debug(DEBUG_DEBUG, "ELF Dump DynSym(%d)\n", h->numDynSym);
        for (int i=0; i<h->numDynSym; ++i)
            printf_debug(DEBUG_DEBUG, "  DynSym[%d] = \"%s\", value=%p, size=%d, info/other=%d/%d index=%d\n", 
                i, h->DynStr+h->DynSym[i].st_name, h->DynSym[i].st_value, h->DynSym[i].st_size,
                h->DynSym[i].st_info, h->DynSym[i].st_other, h->DynSym[i].st_shndx);
        printf_debug(DEBUG_DEBUG, "ELF Dump DynSym=====\n", h->numDynSym);
    }

    return h;
}

void FreeElfHeader(void** head)
{
    elfheader_t *h =(elfheader_t*)(*head);
    free(h->PHEntries);
    free(h->SHEntries);
    free(h->SHStrTab);
    free(h->StrTab);
    free(h->DynStr);
    free(h->SymTab);
    free(h->DynSym);
    free(h);

    *head = NULL;
}
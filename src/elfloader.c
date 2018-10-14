#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>

#include "box86version.h"
#include "elfloader.h"
#include "debug.h"

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

    if(exec)
        if(header.e_type != ET_EXEC) {
            printf_debug(DEBUG_INFO, "Not an Executable (%d)\n", header.e_type);
            return NULL;
    } else {
        if(header.e_type != ET_DYN) {
            printf_debug(DEBUG_INFO, "Not an Library (%d)\n", header.e_type);
            return NULL;
    }

    if(header.e_machine != EM_386) {
        printf_debug(DEBUG_INFO, "Not an i386 ELF (%d)\n", header.e_machine);
        return NULL;
    }

    if(header.e_entry == 0) {
        printf_debug(DEBUG_INFO, "No entry point in ELF\n");
        return NULL;
    }

    printf_debug(DEBUG_DEBUG, "ELF Dump main header\n");
    printf_debug(DEBUG_DEBUG, "  Entry point = %p\n", header.e_entry);
    printf_debug(DEBUG_DEBUG, "  Program Header table offset = %p\n", header.e_phoff);
    printf_debug(DEBUG_DEBUG, "  Section Header table offset = %p\n", header.e_shoff);
    printf_debug(DEBUG_DEBUG, "  Flags = 0x%X\n", header.e_flags);
    printf_debug(DEBUG_DEBUG, "  ELF Header size = %d\n", header.e_ehsize);
    printf_debug(DEBUG_DEBUG, "  Program Header Entry num/size = %d/%d\n", header.e_phnum, header.e_phentsize);
    printf_debug(DEBUG_DEBUG, "  Section Header Entry num/size = %d/%d\n", header.e_shnum, header.e_shentsize);
    printf_debug(DEBUG_DEBUG, "  Section Header index = 0x%X\n", header.e_shstrndx);
    printf_debug(DEBUG_DEBUG, "ELF Dump ==========\n");

    void* ret = malloc(sizeof(Elf32_Ehdr));
    memcpy(ret, &header, sizeof(Elf32_Ehdr));
    return ret;
}

#ifndef __ELFLOADER_PRIVATE_H_
#define __ELFLOADER_PRIVATE_H_

#include <pthread.h>

struct elfheader_s {
    char*       name;
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
    Elf32_Dyn*  Dynamic;
    int         numDynamic;
    char*       DynStrTab;
    int         szDynStrTab;

    intptr_t    delta;  // should be 0

    uintptr_t   entrypoint;
    uintptr_t   initentry;
    uintptr_t   initarray;
    int         initarray_sz;
    uintptr_t   finientry;
    uintptr_t   finiarray;
    int         finiarray_sz;

    uintptr_t   rel;
    int         relsz;
    int         relent;
    uintptr_t   rela;
    int         relasz;
    int         relaent;
    uintptr_t   jmprel;
    int         pltsz;
    int         pltent;
    uint32_t    pltrel;
    uintptr_t   gotplt;
    uintptr_t   got;

    uintptr_t   paddr;
    uintptr_t   vaddr;
    int         align;
    uint32_t    memsz;
    uint32_t    stacksz;
    int         stackalign;
    uint32_t    tlssize;
    int         tlsalign;
    pthread_key_t tlskey;

    int         init_done;
    int         fini_done;

    char*       memory; // char* and not void* to allow math on memory pointer
    char*       tlsdata;
};

#define R_386_NONE	0
#define R_386_32	1
#define R_386_PC32	2
#define R_386_GOT32	3
#define R_386_PLT32	4
#define R_386_COPY	5
#define R_386_GLOB_DAT	6
#define R_386_JMP_SLOT	7
#define R_386_RELATIVE	8
#define R_386_GOTOFF	9
#define R_386_GOTPC	10

#endif //__ELFLOADER_PRIVATE_H_
#ifndef __ELFLOADER_PRIVATE_H_
#define __ELFLOADER_PRIVATE_H_

#ifdef DYNAREC
typedef struct dynablocklist_s dynablocklist_t;
#endif

typedef struct library_s library_t;
typedef struct needed_libs_s needed_libs_t;

#include <pthread.h>

struct elfheader_s {
    char*       name;
    char*       path;   // Resolved path to file
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
    uintptr_t   gotplt_end;
    uintptr_t   pltgot;
    uintptr_t   got;
    uintptr_t   got_end;
    uintptr_t   plt;
    uintptr_t   plt_end;
    uintptr_t   text;
    int         textsz;

    uintptr_t   paddr;
    uintptr_t   vaddr;
    int         align;
    uint32_t    memsz;
    uint32_t    stacksz;
    int         stackalign;
    uint32_t    tlssize;
    int         tlsalign;

    int32_t     tlsbase;    // the base of the tlsdata in the global tlsdata (always negative)

    int         init_done;
    int         fini_done;

    char*       memory; // char* and not void* to allow math on memory pointer
    void**      multiblock;
    uintptr_t*  multiblock_offs;
    uint32_t*   multiblock_size;
    int         multiblock_n;

    library_t   *lib;
    needed_libs_t *neededlibs;
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

elfheader_t* ParseElfHeader(FILE* f, const char* name, int exec);

#endif //__ELFLOADER_PRIVATE_H_
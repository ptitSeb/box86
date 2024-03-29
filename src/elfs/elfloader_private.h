#ifndef __ELFLOADER_PRIVATE_H_
#define __ELFLOADER_PRIVATE_H_

typedef struct library_s library_t;
typedef struct needed_libs_s needed_libs_t;
typedef struct kh_mapsymbols_s kh_mapsymbols_t;

#include <elf.h>
#include "elfloader.h"


typedef struct multiblock_s {
    void*       p;
    uintptr_t   offs;
    uintptr_t   paddr;
    uintptr_t   align;
    size_t      size;
    size_t      asize;
    uint8_t     flags;
} multiblock_t;

typedef struct elfheader_s {
    char*       name;
    char*       path;   // Resolved path to file
    char*       soname; // soname of the elf
    uint16_t    numPHEntries;
    Elf32_Phdr  *PHEntries;
    uint16_t    numSHEntries;
    Elf32_Shdr  *SHEntries;
    int         SHIdx;
    int         numSST;
    char*       SHStrTab;
    char*       StrTab;
    Elf32_Sym*  SymTab;
    uint32_t    numSymTab;
    char*       DynStr;
    Elf32_Sym*  DynSym;
    uint32_t    numDynSym;
    Elf32_Dyn*  Dynamic;
    uint32_t    numDynamic;
    char*       DynStrTab;
    int         szDynStrTab;
    Elf32_Half* VerSym;
    Elf32_Verneed*  VerNeed;
    int         szVerNeed;
    Elf32_Verdef*   VerDef;
    int         szVerDef;
    int         e_type;
    uint32_t    flags;

    intptr_t    delta;  // should be 0
    void*       image;
    void*       raw;    // raw pointer (might be unaligned vs image and max_align of elf)
    size_t      raw_size;

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
    uintptr_t   bss;
    int         bsssz;
    uintptr_t   ehframe;
    uintptr_t   ehframe_end;
    uintptr_t   ehframehdr;

    uintptr_t   paddr;
    uintptr_t   vaddr;
    uint32_t    align;
    uint32_t    memsz;
    uint32_t    reserve;
    uint32_t    stacksz;
    uint32_t    stackalign;
    uintptr_t   tlsaddr;
    uint32_t    tlssize;
    uint32_t    tlsfilesize;
    uint32_t    tlsalign;

    int32_t     tlsbase;    // the base of the tlsdata in the global tlsdata (always negative)

    int         init_done;
    int         fini_done;
    int         refcnt;     // ref count for the elf
    int         malloc_hook_2;  // this elf hook malloc, hacking it

    char*       memory; // char* and not void* to allow math on memory pointer
    multiblock_t*  multiblocks;
    int         multiblock_n;

    library_t   *lib;
    needed_libs_t *needed;

    FILE*       file;
    int         fileno;

    kh_mapsymbols_t   *mapsymbols;
    kh_mapsymbols_t   *weaksymbols;
    kh_mapsymbols_t   *localsymbols;
    kh_defaultversion_t *globaldefver;  // the global default version for symbols (the XXX@@vvvv of symbols)
    kh_defaultversion_t *weakdefver;    // the weak default version for symbols (the XXX@@vvvv of symbols)
} elfheader_t;

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

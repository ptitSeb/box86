#ifndef __ELFLOADER_PRIVATE_H_
#define __ELFLOADER_PRIVATE_H_

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

    uintptr_t   paddr;
    uintptr_t   vaddr;
    int         align;
    uint32_t    memsz;
    uint32_t    stacksz;
    int         stackalign;

    char*       memory; // char* and not void* to allow math on memory pointer
};

#endif //__ELFLOADER_PRIVATE_H_
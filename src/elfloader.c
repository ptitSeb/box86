#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include <sys/mman.h>

#include "box86version.h"
#include "elfloader.h"
#include "debug.h"
#include "elfload_dump.h"
#include "elfloader_private.h"

void FreeElfHeader(elfheader_t** head)
{
    if(!head || !*head)
        return;
    elfheader_t *h = *head;
    free(h->name);
    free(h->PHEntries);
    free(h->SHEntries);
    free(h->SHStrTab);
    free(h->StrTab);
    free(h->DynStr);
    free(h->SymTab);
    free(h->DynSym);
    free(h->memory);
    free(h);

    *head = NULL;
}

int CalcLoadAddr(elfheader_t* head)
{
    head->memsz = 0;
    head->paddr = head->vaddr = (uintptr_t)~0;
    head->align = 1;
    for (int i=0; i<head->numPHEntries; ++i)
        if(head->PHEntries[i].p_type == PT_LOAD) {
            if(head->paddr > head->PHEntries[i].p_paddr)
                head->paddr = head->PHEntries[i].p_paddr;
            if(head->vaddr > head->PHEntries[i].p_vaddr)
                head->vaddr = head->PHEntries[i].p_vaddr;
        }
    
    if(head->vaddr==~0 || head->paddr==~0) {
        printf_debug(DEBUG_NONE, "Error: v/p Addr for Elf Load not set\n");
        return 1;
    }

    head->stacksz = 1024*1024;          //1M stack size default?
    head->stackalign = 4;   // default align for stack
    for (int i=0; i<head->numPHEntries; ++i) {
        if(head->PHEntries[i].p_type == PT_LOAD) {
            uintptr_t phend = head->PHEntries[i].p_vaddr - head->vaddr + head->PHEntries[i].p_memsz;
            if(phend > head->memsz)
                head->memsz = phend;
            if(head->PHEntries[i].p_align > head->align)
                head->align = head->PHEntries[i].p_align;
        }
        if(head->PHEntries[i].p_type == PT_GNU_STACK) {
            if(head->stacksz < head->PHEntries[i].p_memsz)
                head->stacksz = head->PHEntries[i].p_memsz;
            if(head->stackalign < head->PHEntries[i].p_align)
                head->stackalign = head->PHEntries[i].p_align;
        }
    }
    printf_debug(DEBUG_DEBUG, "Elf Addr(v/p)=%p/%p Memsize=0x%x (align=0x%x)\n", head->vaddr, head->paddr, head->memsz, head->align);
    printf_debug(DEBUG_DEBUG, "Elf Stack Memsize=%u (align=%u)\n", head->stacksz, head->stackalign);

    return 0;
}

const char* ElfName(elfheader_t* head)
{
    return head->name;
}

int AllocElfMemory(elfheader_t* head)
{
    printf_debug(DEBUG_DEBUG, "Allocating memory for Elf \"%s\"\n", head->name);
    if (posix_memalign((void**)&head->memory, head->align, head->memsz)) {
        printf_debug(DEBUG_NONE, "Cannot allocate aligned memory (0x%x/0x%x) for elf \"%s\"\n", head->memsz, head->align, head->name);
        return 1;
    }
    printf_debug(DEBUG_DEBUG, "Address is %p\n", head->memory);
    printf_debug(DEBUG_DEBUG, "And setting memory access to PROT_READ | PROT_WRITE | PROT_EXEC\n");
    if (mprotect(head->memory, head->memsz, PROT_READ | PROT_WRITE | PROT_EXEC)) {
        printf_debug(DEBUG_NONE, "Cannot protect memory for elf \"%s\"\n", head->name);
        // memory protect error not fatal for now....
    }

    return 0;
}

int LoadElfMemory(FILE* f, elfheader_t* head)
{
    for (int i=0; i<head->numPHEntries; ++i) {
        if(head->PHEntries[i].p_type == PT_LOAD) {
            Elf32_Phdr * e = &head->PHEntries[i];
            char* dest = head->memory + e->p_paddr - head->paddr;
            printf_debug(DEBUG_DEBUG, "Loading block #%i @%p (0x%x/0x%x)\n", i, dest, e->p_filesz, e->p_memsz);
            fseek(f, e->p_offset, SEEK_SET);
            if(fread(dest, e->p_filesz, 1, f)!=1) {
                printf_debug(DEBUG_NONE, "Fail to read PT_LOAD part #%d\n", i);
                return 1;
            }
            // zero'd difference between filesz and memsz
            if(e->p_filesz != e->p_memsz)
                memset(dest+e->p_filesz, 0, e->p_memsz - e->p_filesz);
        }
    }
    return 0;
}

int RelocateElf(elfheader_t* head)
{
    if(head->rel) {
        int cnt = head->relsz / head->relent;
        DumpRelTable(head);
        printf_debug(DEBUG_DEBUG, "Applying %d Rellocation(s)\n", cnt);
        Elf32_Rel *rel = (Elf32_Rel *)(head->memory + head->rel - head->paddr);
        for (int i=0; i<cnt; ++i) {
            Elf32_Sym *sym = &head->DynSym[ELF32_R_SYM(rel[i].r_info)];
            uint32_t *p = (uint32_t*)(head->memory + rel[i].r_offset - head->paddr);
            int t = ELF32_R_TYPE(rel[i].r_info);
            switch(t) {
                case R_386_NONE:
                case R_386_PC32:
                    // can be ignored
                    printf_debug(DEBUG_DEBUG, "Ignoring %s @%p (%p)\n", DumpRelType(t), p, p?(*p):0);
                    break;
                case R_386_GLOB_DAT:
                    // I guess it can be ignored
                    printf_debug(DEBUG_DEBUG, "Ignoring %s @%p (%p)\n", DumpRelType(t), p);
                    break;
                case R_386_RELATIVE:
                    printf_debug(DEBUG_DEBUG, "Apply R_386_RELATIVE @%p (%p -> %p)\n", p, *p, (*p)+(uintptr_t)head->memory - head->paddr);
                    *p += (uintptr_t)head->memory - head->paddr;
                    break;
                case R_386_32:
                    printf_debug(DEBUG_DEBUG, "Apply R_386_32 @%p with sym=%s (%p -> %p)\n", p, DumpSym(head, sym), *p, *p);
                    return -1; //TODO!!!
                    //break;
                default:
                    printf_debug(DEBUG_INFO, "Warning, don't know of to handle rel #%d %s\n", i, DumpRelType(ELF32_R_TYPE(rel[i].r_info)));
            }
        }
    }
    if(head->rela) {
        int cnt = head->relasz / head->relaent;
        DumpRelATable(head);
        printf_debug(DEBUG_DEBUG, "Applying %d Rellocation(s) with Addend\n", cnt);
        Elf32_Rela *rela = (Elf32_Rela *)(head->memory + head->rela - head->paddr);
        for (int i=0; i<cnt; ++i) {
            Elf32_Sym *sym = &head->DynSym[ELF32_R_SYM(rela[i].r_info)];
            switch(ELF32_R_TYPE(rela[i].r_info)) {
                case R_386_NONE:
                case R_386_PC32:
                    // can be ignored
                    break;
                
                default:
                    printf_debug(DEBUG_INFO, "Warning, don't know of to handle rel #%d %s\n", i, DumpRelType(ELF32_R_TYPE(rela[i].r_info)));
            }
        }
    }
   
    return 0;
}

void CalcStack(elfheader_t* elf, uint32_t* stacksz, int* stackalign)
{
    if(*stacksz < elf->stacksz)
        *stacksz = elf->stacksz;
    if(*stackalign < elf->stackalign)
        *stackalign = elf->stackalign;
}
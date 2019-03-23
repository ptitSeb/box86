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
#include "librarian.h"
#include "x86run.h"
#include "bridge.h"
#include "wrapper.h"
#include "box86context.h"
#include "library.h"
#include "x86emu.h"
#include "stack.h"

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
    free(h->Dynamic);
    free(h->DynStr);
    free(h->SymTab);
    free(h->DynSym);
    FreeElfMemory(h);
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
        printf_log(LOG_NONE, "Error: v/p Addr for Elf Load not set\n");
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
    printf_log(LOG_DEBUG, "Elf Addr(v/p)=%p/%p Memsize=0x%x (align=0x%x)\n", (void*)head->vaddr, (void*)head->paddr, head->memsz, head->align);
    printf_log(LOG_DEBUG, "Elf Stack Memsize=%u (align=%u)\n", head->stacksz, head->stackalign);

    return 0;
}

const char* ElfName(elfheader_t* head)
{
    return head->name;
}

int AllocElfMemory(elfheader_t* head, int mainbin)
{
    uintptr_t offs = 0;
    if(mainbin && head->vaddr==0) {
        char* load_addr = getenv("BOX86_LOAD_ADDR");
        if(load_addr)
            if(sscanf(load_addr, "0x%x", &offs)!=1)
                offs = 0;
    }
    if(!offs)
        offs = head->vaddr;
    printf_log(LOG_DEBUG, "Allocating 0x%x memory @%p for Elf \"%s\"\n", head->memsz, (void*)offs, head->name);
    void* p = mmap((void*)offs, head->memsz
        , PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED | MAP_ANONYMOUS | ((offs)?MAP_FIXED:0)
        , -1, 0);
    if(p==MAP_FAILED) {
        printf_log(LOG_NONE, "Cannot create memory map (@%p 0x%x/0x%x) for elf \"%s\"\n", (void*)offs, head->memsz, head->align, head->name);
        return 1;
    }
    head->memory = p;
    memset(p, 0, head->memsz);
    head->delta = (intptr_t)p - (intptr_t)head->vaddr;
    printf_log(LOG_DEBUG, "Got %p (delta=%p)\n", p, (void*)head->delta);

    return 0;
}

void FreeElfMemory(elfheader_t* head)
{
    if(head->memory)
        munmap((void*)head->memory, head->memsz);
}

int LoadElfMemory(FILE* f, elfheader_t* head)
{
    for (int i=0; i<head->numPHEntries; ++i) {
        if(head->PHEntries[i].p_type == PT_LOAD) {
            Elf32_Phdr * e = &head->PHEntries[i];
            char* dest = (char*)e->p_paddr + head->delta;
            printf_log(LOG_DEBUG, "Loading block #%i @%p (0x%x/0x%x)\n", i, dest, e->p_filesz, e->p_memsz);
            fseek(f, e->p_offset, SEEK_SET);
            if(fread(dest, e->p_filesz, 1, f)!=1) {
                printf_log(LOG_NONE, "Fail to read PT_LOAD part #%d\n", i);
                return 1;
            }
            // zero'd difference between filesz and memsz
            if(e->p_filesz != e->p_memsz)
                memset(dest+e->p_filesz, 0, e->p_memsz - e->p_filesz);
        }
    }
    return 0;
}

int RelocateElfREL(lib_t *maplib, elfheader_t* head, int cnt, Elf32_Rel *rel)
{
    for (int i=0; i<cnt; ++i) {
        Elf32_Sym *sym = &head->DynSym[ELF32_R_SYM(rel[i].r_info)];
        int type = ELF32_ST_TYPE(sym->st_info);
        int bind = ELF32_ST_BIND(sym->st_info);
        const char* symname = SymName(head, sym);
        uint32_t *p = (uint32_t*)(rel[i].r_offset + head->delta);
        uintptr_t offs = 0;
        uint32_t sz = 0;
        uintptr_t end = 0;
        if(bind==STB_LOCAL)
            GetLocalSymbolStartEnd(maplib, symname, &offs, &end, head);
        else
            GetGlobalSymbolStartEnd(maplib, symname, &offs, &end);
        uintptr_t globoffs, globend;
        int t = ELF32_R_TYPE(rel[i].r_info);
        switch(t) {
            case R_386_NONE:
                // can be ignored
                printf_log(LOG_DEBUG, "Ignoring %s @%p (%p)\n", DumpRelType(t), p, (void*)(p?(*p):0));
                break;
            case R_386_PC32:
                    if (!offs) {
                        printf_log(LOG_NONE, "Error: Global Symbol %s not found, cannot apply R_386_PC32 @%p (%p)\n", symname, p, *(void**)p);
                    }
                    offs = (offs - (uintptr_t)p);
                    if(!offs)
                    printf_log(LOG_DEBUG, "Apply R_386_PC32 @%p with sym=%s (%p -> %p)\n", p, symname, *(void**)p, (void*)(*(uintptr_t*)p+offs));
                    *p += offs;
                break;
            case R_386_GLOB_DAT:
                // Look for same symbol already loaded but not in self
                if (GetGlobalNoWeakSymbolStartEnd(maplib, symname, &globoffs, &globend)) {
                    offs = globoffs;
                    end = globend;
                }
                if (!offs) {
                    printf_log(LOG_NONE, "Error: Global Symbol %s not found, cannot apply R_386_GLOB_DAT @%p (%p)\n", symname, p, *(void**)p);
//                    return -1;
                } else {
                    printf_log(LOG_DEBUG, "Apply R_386_GLOB_DAT @%p (%p -> %p) on sym=%s\n", p, (void*)(p?(*p):0), (void*)offs, symname);
                    *p = offs;
                }
                break;
            case R_386_RELATIVE:
                printf_log(LOG_DEBUG, "Apply R_386_RELATIVE @%p (%p -> %p)\n", p, *(void**)p, (void*)((*p)+head->delta));
                *p += head->delta;
                break;
            case R_386_32:
                if (!offs) {
                    printf_log(LOG_NONE, "Error: Symbol %s not found, cannot apply R_386_32 @%p (%p)\n", symname, p, *(void**)p);
//                    return -1;
                } else {
                    printf_log(LOG_DEBUG, "Apply R_386_32 @%p with sym=%s (%p -> %p)\n", p, symname, *(void**)p, (void*)(offs+*(uint32_t*)p));
                    *p += offs;
                }
                break;
            case R_386_TLS_DTPMOD32:
                // try to use _dl_next_tls_modid() ?
                // keeping 0 there should work, as a "current module" information?
                break;
            case R_386_TLS_DTPOFF32:
            case R_386_JMP_SLOT:
                if (!offs) {
                    printf_log(LOG_NONE, "Error: Symbol %s not found, cannot apply %s @%p (%p)\n", symname, (t==R_386_JMP_SLOT)?"R_386_JMP_SLOT":"R_386_TLS_DTPOFF32", p, *(void**)p);
//                    return -1;
                } else {
                    if(p) {
                        printf_log(LOG_DEBUG, "Apply %s @%p with sym=%s (%p -> %p)\n", (t==R_386_JMP_SLOT)?"R_386_JMP_SLOT":"R_386_TLS_DTPOFF32", p, symname, *(void**)p, (void*)offs);
                        *p = offs;
                    } else {
                        printf_log(LOG_NONE, "Warning, Symbol %s found, but Jump Slot Offset is NULL \n", symname);
                    }
                }
                break;
            case R_386_COPY:
                if(offs) {
                    GetNoSelfSymbolStartEnd(maplib, symname, &offs, &end, head);   // get original copy if any
                    printf_log(LOG_DEBUG, "Apply R_386_COPY @%p with sym=%s, @%p size=%d (", p, symname, (void*)offs, sym->st_size);
                    memmove(p, (void*)offs, sym->st_size);
                    if(LOG_DEBUG<=box86_log) {
                        uint32_t*k = (uint32_t*)p;
                        for (int i=0; i<sym->st_size; i+=4, ++k)
                            printf_log(LOG_DEBUG, "%s0x%08X", i?" ":"", *k);
                        printf_log(LOG_DEBUG, ")\n");
                    }
                } else {
                    printf_log(LOG_NONE, "Error: Symbol %s not found, cannot apply R_386_COPY @%p (%p)\n", symname, p, *(void**)p);
                }
                break;
            default:
                printf_log(LOG_INFO, "Warning, don't know of to handle rel #%d %s (%p)\n", i, DumpRelType(ELF32_R_TYPE(rel[i].r_info)), p);
        }
    }
    return 0;
}

int RelocateElfRELA(lib_t *maplib, elfheader_t* head, int cnt, Elf32_Rela *rela)
{
    for (int i=0; i<cnt; ++i) {
        Elf32_Sym *sym = &head->DynSym[ELF32_R_SYM(rela[i].r_info)];
        const char* symname = SymName(head, sym);
        uint32_t *p = (uint32_t*)(rela[i].r_offset + head->delta);
        uintptr_t offs = 0;
        uint32_t sz = 0;
        uintptr_t end = 0;
        switch(ELF32_R_TYPE(rela[i].r_info)) {
            case R_386_NONE:
            case R_386_PC32:
                // can be ignored
                break;
            case R_386_COPY:
                GetNoSelfSymbolStartEnd(maplib, symname, &offs, &end, head);
                if(offs) {
                    // add r_addend to p?
                    printf_log(LOG_DEBUG, "Apply R_386_COPY @%p with sym=%s, @%p size=%d\n", p, symname, (void*)offs, sym->st_size);
                    memcpy(p, (void*)(offs+rela[i].r_addend), sym->st_size);
                } else {
                    printf_log(LOG_NONE, "Error: Symbol %s not found, cannot apply R_386_COPY @%p (%p)\n", symname, p, *(void**)p);
                }
                break;
            default:
                printf_log(LOG_INFO, "Warning, don't know of to handle rela #%d %s on %s\n", i, DumpRelType(ELF32_R_TYPE(rela[i].r_info)), symname);
        }
    }
    return 0;
}
int RelocateElf(lib_t *maplib, elfheader_t* head)
{
    if(head->rel) {
        int cnt = head->relsz / head->relent;
        DumpRelTable(head, cnt, (Elf32_Rel *)(head->rel + head->delta), "Rel");
        printf_log(LOG_DEBUG, "Applying %d Relocation(s) for %s\n", cnt, head->name);
        if(RelocateElfREL(maplib, head, cnt, (Elf32_Rel *)(head->rel + head->delta)))
            return -1;
    }
    if(head->rela) {
        int cnt = head->relasz / head->relaent;
        DumpRelATable(head, cnt, (Elf32_Rela *)(head->rela + head->delta), "RelA");
        printf_log(LOG_DEBUG, "Applying %d Relocation(s) with Addend for %s\n", cnt, head->name);
        if(RelocateElfRELA(maplib, head, cnt, (Elf32_Rela *)(head->rela + head->delta)))
            return -1;
    }
   
    return 0;
}

int RelocateElfPlt(box86context_t* context, lib_t *maplib, elfheader_t* head)
{
    if(head->pltrel) {
        int cnt = head->pltsz / head->pltent;
        if(head->pltrel==DT_REL) {
            DumpRelTable(head, cnt, (Elf32_Rel *)(head->jmprel + head->delta), "PLT");
            printf_log(LOG_DEBUG, "Applying %d PLT Relocation(s) for %s\n", cnt, head->name);
            if(RelocateElfREL(maplib, head, cnt, (Elf32_Rel *)(head->jmprel + head->delta)))
                return -1;
        } else if(head->pltrel==DT_RELA) {
            DumpRelATable(head, cnt, (Elf32_Rela *)(head->jmprel + head->delta), "PLT");
            printf_log(LOG_DEBUG, "Applying %d PLT Relocation(s) with Addend for %s\n", cnt, head->name);
            if(RelocateElfRELA(maplib, head, cnt, (Elf32_Rela *)(head->jmprel + head->delta)))
                return -1;
        }
        if(head->gotplt) {
            if(pltResolver==~0) {
                pltResolver = AddBridge(context->system, vFEpp, PltResolver, 0);   // should be vFEuu
            }
            *(uintptr_t*)(head->gotplt+head->delta+8) = pltResolver;
            printf_log(LOG_DEBUG, "PLT Resolver injected at %p\n", (void*)(head->gotplt+head->delta+8));
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

Elf32_Sym* GetFunction(elfheader_t* h, const char* name)
{
    // TODO: create a hash on named to avoid this loop
    for (int i=0; i<h->numSymTab; ++i) {
        if(h->SymTab[i].st_info == 18) {    // TODO: this "18" is probably defined somewhere
            const char * symname = h->StrTab+h->SymTab[i].st_name;
            if(strcmp(symname, name)==0) {
                return h->SymTab+i;
            }
        }
    }
    return NULL;
}

uintptr_t GetFunctionAddress(elfheader_t* h, const char* name)
{
    Elf32_Sym* sym = GetFunction(h, name);
    if(sym) return sym->st_value;
    return 0;
}

uintptr_t GetEntryPoint(lib_t* maplib, elfheader_t* h)
{
    uintptr_t ep = h->entrypoint + h->delta;
    printf_log(LOG_DEBUG, "Entry Point is %p\n", (void*)ep);
    if(box86_log>=LOG_DUMP) {
        printf_log(LOG_DUMP, "(short) Dump of Entry point\n");
        int sz = 64;
        uintptr_t lastbyte = GetLastByte(h);
        if (ep + sz >  lastbyte)
            sz = lastbyte - ep;
        DumpBinary((char*)ep, sz);
    }
    /*
    // but instead of regular entrypoint, lets grab "main", it will be easier to manage I guess
    uintptr_t m = FindSymbol(maplib, "main");
    if(m) {
        ep = m;
        printf_log(LOG_DEBUG, "Using \"main\" as Entry Point @%p\n", ep);
        if(box86_log>=LOG_DUMP) {
            printf_log(LOG_DUMP, "(short) Dump of Entry point\n");
            int sz = 64;
            uintptr_t lastbyte = GetLastByte(h);
            if (ep + sz >  lastbyte)
                sz = lastbyte - ep;
            DumpBinary((char*)ep, sz);
        }
    }
    */
    return ep;
}

uintptr_t GetLastByte(elfheader_t* h)
{
    return (uintptr_t)h->memory + h->delta + h->memsz;
}

void AddSymbols(lib_t *maplib, kh_mapsymbols_t* mapsymbols, kh_mapsymbols_t* weaksymbols, kh_mapsymbols_t* localsymbols, elfheader_t* h)
{
    printf_log(LOG_DUMP, "Will look for Symbol to add in SymTable(%d)\n", h->numSymTab);
    for (int i=0; i<h->numSymTab; ++i) {
        const char * symname = h->StrTab+h->SymTab[i].st_name;
        int bind = ELF32_ST_BIND(h->SymTab[i].st_info);
        int type = ELF32_ST_TYPE(h->SymTab[i].st_info);
        #if 0
        if(((h->SymTab[i].st_info & 2) || (h->SymTab[i].st_info == 17)
             || (h->SymTab[i].st_info == 161 && !FindGlobalSymbol(maplib, symname)) 
             || (h->SymTab[i].st_info == 33)
            )
            && (h->SymTab[i].st_other==0) && (h->SymTab[i].st_shndx!=0)) {
            if(!h->SymTab[i].st_value && FindGlobalSymbol(maplib, symname))
                continue;
        #else
        if((type==STT_OBJECT || type==STT_FUNC || type==STT_COMMON || type==STT_TLS) 
        && (h->SymTab[i].st_other==0) && (h->SymTab[i].st_shndx!=0)) {
            if(bind==10/*STB_GNU_UNIQUE*/ && FindGlobalSymbol(maplib, symname))
                continue;
        #endif
            uintptr_t offs = h->SymTab[i].st_value + h->delta;
            uint32_t sz = h->SymTab[i].st_size;
            printf_log(LOG_DUMP, "Adding Symbol(bind=%d) \"%s\" with offset=%p sz=%d\n", bind, symname, (void*)offs, sz);
            if(bind==STB_LOCAL)
                AddSymbol(localsymbols, symname, offs, sz);
            else if(bind==STB_WEAK)
                AddSymbol(weaksymbols, symname, offs, sz);
            else
                AddSymbol(mapsymbols, symname, offs, sz);
        }
    }
    
    printf_log(LOG_DUMP, "Will look for Symbol to add in DynSym (%d)\n", h->numDynSym);
    for (int i=0; i<h->numDynSym; ++i) {
        const char * symname = h->DynStr+h->DynSym[i].st_name;
        int bind = ELF32_ST_BIND(h->DynSym[i].st_info);
        int type = ELF32_ST_TYPE(h->DynSym[i].st_info);
        #if 0
        if(((h->DynSym[i].st_info & 2) || (h->DynSym[i].st_info == 17)
             || (h->DynSym[i].st_info == 161 && !FindGlobalSymbol(maplib, symname))
             || (h->DynSym[i].st_info == 33) 
            )
            && (h->DynSym[i].st_other==0) && (h->DynSym[i].st_shndx!=0 && h->DynSym[i].st_shndx<62521)) {
            if(!h->DynSym[i].st_value && FindGlobalSymbol(maplib, symname))
                continue;
        #else
        if((type==STT_OBJECT || type==STT_FUNC || type==STT_COMMON || type==STT_TLS) 
        && (h->DynSym[i].st_other==0) && (h->DynSym[i].st_shndx!=0 && h->DynSym[i].st_shndx<62521)) {
            if(bind==10/*STB_GNU_UNIQUE*/ && FindGlobalSymbol(maplib, symname))
                continue;
        #endif
            uintptr_t offs = h->DynSym[i].st_value + h->delta;
            uint32_t sz = h->DynSym[i].st_size;
            printf_log(LOG_DUMP, "Adding Symbol(bind=%d) \"%s\" with offset=%p sz=%d\n", bind, symname, (void*)offs, sz);
            if(bind==STB_LOCAL)
                AddSymbol(localsymbols, symname, offs, sz);
            else if(bind==STB_WEAK)
                AddSymbol(weaksymbols, symname, offs, sz);
            else
                AddSymbol(mapsymbols, symname, offs, sz);
        }
    }
    
}

/*
$ORIGIN – Provides the directory the object was loaded from. This token is typical
used for locating dependencies in unbundled packages. For more details of this
token expansion, see “Locating Associated Dependencies”
$OSNAME – Expands to the name of the operating system (see the uname(1) man
page description of the -s option). For more details of this token expansion, see
“System Specific Shared Objects”
$OSREL – Expands to the operating system release level (see the uname(1) man
page description of the -r option). For more details of this token expansion, see
“System Specific Shared Objects”
$PLATFORM – Expands to the processor type of the current machine (see the
uname(1) man page description of the -i option). For more details of this token
expansion, see “System Specific Shared Objects”
*/
int LoadNeededLibs(elfheader_t* h, lib_t *maplib, box86context_t *box86, x86emu_t* emu)
{
   DumpDynamicNeeded(h);
   for (int i=0; i<h->numDynamic; ++i)
        if(h->Dynamic[i].d_tag==DT_NEEDED) {
            char *needed = h->DynStrTab+h->delta+h->Dynamic[i].d_un.d_val;
            // TODO: Add LD_LIBRARY_PATH and RPATH Handling
            if(AddNeededLib(maplib, needed, box86, emu)) {
                printf_log(LOG_INFO, "Error loading needed lib: \"%s\"\n", needed);
                return 1;   //error...
            }
        }
    return 0;
}
int FinalizeNeededLibs(elfheader_t* h, lib_t *maplib, box86context_t *box86, x86emu_t* emu)
{
   DumpDynamicNeeded(h);
   for (int i=0; i<h->numDynamic; ++i)
        if(h->Dynamic[i].d_tag==DT_NEEDED) {
            char *needed = h->DynStrTab+h->delta+h->Dynamic[i].d_un.d_val;
            // TODO: Add LD_LIBRARY_PATH and RPATH Handling
            if(FinalizeLibrary(GetLib(maplib, needed), emu)) {
                printf_log(LOG_INFO, "Error finalizing needed lib: \"%s\"\n", needed);
                return 1;   //error...
            }
        }
    return 0;
}

void RunElfInit(elfheader_t* h, x86emu_t *emu)
{
    if(h->init_done || !h->initentry)
        return;
    uintptr_t p = h->initentry + h->delta;
    box86context_t* context = GetEmuContext(emu);
    if(context->deferedInit) {
        if(context->deferedInitSz==context->deferedInitCap) {
            context->deferedInitCap += 4;
            context->deferedInitList = (elfheader_t**)realloc(context->deferedInitList, context->deferedInitCap*sizeof(elfheader_t*));
        }
        context->deferedInitList[context->deferedInitSz++] = h;
        return;
    }
    printf_log(LOG_DEBUG, "Calling Init for %s @%p\n", ElfName(h), (void*)p);
    EmuCall(emu, p);    // should be sure that argc, argv and env are pushed to the stack!
    // and check init array now
    Elf32_Addr *addr = (Elf32_Addr*)(h->initarray + h->delta);
    for (int i=0; i<h->initarray_sz; ++i) {
        printf_log(LOG_DEBUG, "Calling Init[%d] for %s @%p\n", i, ElfName(h), (void*)addr[i]);
        EmuCall(emu, (uintptr_t)addr[i]);
    }
    h->init_done = 1;
    return;
}
void RunDeferedElfInit(x86emu_t *emu)
{
    box86context_t* context = GetEmuContext(emu);
    if(!context->deferedInit)
        return;
    context->deferedInit = 0;
    if(!context->deferedInitList)
        return;
    for (int i=0; i<context->deferedInitSz; ++i)
        RunElfInit(context->deferedInitList[i], emu);
    free(context->deferedInitList);
    context->deferedInitList = NULL;
    context->deferedInitCap = context->deferedInitSz = 0;
}
void RunElfFini(elfheader_t* h, x86emu_t *emu)
{
    if(h->fini_done || !h->finientry)
        return;
    uintptr_t p = h->finientry + h->delta;
    printf_log(LOG_DEBUG, "Calling Fini for %s @%p\n", ElfName(h), (void*)p);
    uint32_t sESP = GetESP(emu);
    Push32(emu, (uintptr_t)GetEmuContext(emu)->envv);
    Push32(emu, (uintptr_t)GetEmuContext(emu)->argv);
    Push32(emu, GetEmuContext(emu)->argc);
    EmuCall(emu, p);
    // and check fini array now
    Elf32_Addr *addr = (Elf32_Addr*)(h->finiarray + h->delta);
    for (int i=0; i<h->finiarray_sz; ++i) {
        printf_log(LOG_DEBUG, "Calling Fini[%d] for %s @%p\n", i, ElfName(h), (void*)addr[i]);
        EmuCall(emu, (uintptr_t)addr[i]);
    }
    SetESP(emu, sESP);
    h->fini_done = 1;
    return;
}

uintptr_t GetElfInit(elfheader_t* h)
{
    return h->initentry + h->delta;
}
uintptr_t GetElfFini(elfheader_t* h)
{
    return h->finientry + h->delta;
}

void* GetBaseAddress(elfheader_t* h)
{
    return h->memory;
}

uint32_t GetBaseSize(elfheader_t* h)
{
    return h->memsz;
}

const char* FindNearestSymbolName(elfheader_t* h, void* p, uintptr_t* start, uint32_t* sz)
{
    uintptr_t addr = (uintptr_t)p;

    uint32_t distance = 0x7fffffff;
    const char* ret = NULL;
    uintptr_t s = 0;
    uint32_t size = 0;

    for (int i=0; i<h->numSymTab && distance!=0; ++i) {   
        const char * symname = h->StrTab+h->SymTab[i].st_name;
        uintptr_t offs = h->SymTab[i].st_value + h->delta;

        if(offs<addr) {
            if(distance>addr-offs) {
                distance = addr-offs;
                ret = symname;
                s = offs;
                size = h->SymTab[i].st_size;
            }
        }
    }

    if(start)
        *start = s;
    if(sz)
        *sz = size;

    return ret;
}
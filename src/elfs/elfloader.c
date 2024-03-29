    #define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <link.h>
#include <unistd.h>
#include <errno.h>
#include <dlfcn.h>

#include "custommem.h"
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
#include "box86stack.h"
#include "callback.h"
#include "dynarec.h"
#include "box86stack.h"
#include "wine_tools.h"
#include "dictionnary.h"
#include "symbols.h"
#ifdef DYNAREC
#include "dynablock.h"
#endif
#include "../emu/x86emu_private.h"
#include "x86tls.h"

void* my__IO_2_1_stderr_ = NULL;
void* my__IO_2_1_stdin_  = NULL;
void* my__IO_2_1_stdout_ = NULL;

// return the index of header (-1 if it doesn't exist)
int getElfIndex(box86context_t* ctx, elfheader_t* head) {
    for (int i=0; i<ctx->elfsize; ++i)
        if(ctx->elfs[i]==head)
            return i;
    return -1;
}

elfheader_t* LoadAndCheckElfHeader(FILE* f, const char* name, int exec)
{
    elfheader_t *h = ParseElfHeader(f, name, exec);
    if(!h)
        return NULL;

    if ((h->path = box_realpath(name, NULL)) == NULL) {
        h->path = (char*)box_malloc(1);
        h->path[0] = '\0';
    }
    
    h->mapsymbols = NewMapSymbols();
    h->weaksymbols = NewMapSymbols();
    h->localsymbols = NewMapSymbols();
    h->globaldefver = NewDefaultVersion();
    h->weakdefver = NewDefaultVersion();
    h->refcnt = 1;

    h->file = f;
    h->fileno = fileno(f);
    
    return h;
}

void FreeElfHeader(elfheader_t** head)
{
    if(!head || !*head)
        return;
    elfheader_t *h = *head;
    if(my_context)
        RemoveElfHeader(my_context, h);

    box_free(h->PHEntries);
    box_free(h->SHEntries);
    box_free(h->SHStrTab);
    box_free(h->StrTab);
    box_free(h->Dynamic);
    box_free(h->DynStr);
    box_free(h->SymTab);
    box_free(h->DynSym);

    FreeMapSymbols(&h->mapsymbols);
    FreeMapSymbols(&h->weaksymbols);
    FreeMapSymbols(&h->localsymbols);
    FreeDefaultVersion(&h->globaldefver);
    FreeDefaultVersion(&h->weakdefver);
    
    FreeElfMemory(h);

    box_free(h->name);
    box_free(h->path);
    if(h->file)
        fclose(h->file);
    box_free(h);

    *head = NULL;
}

int CalcLoadAddr(elfheader_t* head)
{
    head->memsz = 0;
    head->paddr = head->vaddr = ~(uintptr_t)0;
    head->align = 1;
    for (int i=0; i<head->numPHEntries; ++i)
        if(head->PHEntries[i].p_type == PT_LOAD) {
            if(head->paddr > (uintptr_t)head->PHEntries[i].p_paddr)
                head->paddr = (uintptr_t)head->PHEntries[i].p_paddr;
            if(head->vaddr > (uintptr_t)head->PHEntries[i].p_vaddr)
                head->vaddr = (uintptr_t)head->PHEntries[i].p_vaddr;
        }
    
    if(head->vaddr==~(uintptr_t)0 || head->paddr==~(uintptr_t)0) {
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
        if(head->PHEntries[i].p_type == PT_TLS) {
            head->tlsaddr = head->PHEntries[i].p_vaddr;
            head->tlssize = head->PHEntries[i].p_memsz;
            head->tlsfilesize = head->PHEntries[i].p_filesz;
            head->tlsalign = head->PHEntries[i].p_align;
            // force alignement...
            if(head->tlsalign>1)
                while(head->tlssize&(head->tlsalign-1))
                    head->tlssize++;
        }
    }
    printf_log(LOG_DEBUG, "Elf Addr(v/p)=%p/%p Memsize=0x%zx (align=0x%zx)\n", (void*)head->vaddr, (void*)head->paddr, head->memsz, head->align);
    printf_log(LOG_DEBUG, "Elf Stack Memsize=%zu (align=%zu)\n", head->stacksz, head->stackalign);
    printf_log(LOG_DEBUG, "Elf TLS Memsize=%zu (align=%zu)\n", head->tlssize, head->tlsalign);

    return 0;
}

const char* ElfName(elfheader_t* head)
{
    if(!head)
        return "(noelf)";
    return head->name;
}
const char* ElfPath(elfheader_t* head)
{
    if(!head)
        return NULL;
    return head->path;
}

int AllocLoadElfMemory(box86context_t* context, elfheader_t* head, int mainbin)
{
    uintptr_t offs = 0;
    loadProtectionFromMap();
    int log_level = box86_load_addr?LOG_INFO:LOG_DEBUG;

    head->multiblock_n = 0; // count PHEntrie with LOAD
    uintptr_t max_align = (box86_pagesize-1);
    for (size_t i=0; i<head->numPHEntries; ++i) 
        if(head->PHEntries[i].p_type == PT_LOAD && head->PHEntries[i].p_flags) {
            ++head->multiblock_n;
        }

    if(!head->vaddr && box86_load_addr) {
        offs = (uintptr_t)findBlockNearHint((void*)box86_load_addr, head->memsz, max_align);
        box86_load_addr = offs + head->memsz;
        box86_load_addr = (box86_load_addr+0x10ffffff)&~0xffffff;
    }
    if(!offs && !head->vaddr)
        offs = (uintptr_t)find32bitBlockElf(head->memsz, mainbin, max_align);
    // prereserve the whole elf image, without populating
    size_t sz = head->memsz;
    void* raw = NULL;
    void* image = NULL;
    if(!head->vaddr) {
        sz += head->align;
        raw = mmap64((void*)offs, sz, 0, MAP_ANONYMOUS|MAP_PRIVATE|MAP_NORESERVE, -1, 0);
        image = (void*)(((uintptr_t)raw+max_align)&~max_align);
    } else {
        image = raw = mmap64((void*)head->vaddr, sz, 0, MAP_ANONYMOUS|MAP_PRIVATE|MAP_NORESERVE, -1, 0);
    }
    if(image!=MAP_FAILED && !head->vaddr && image!=(void*)offs) {
        printf_log(LOG_INFO, "%s: Mmap64 for (@%p 0x%zx) for elf \"%s\" returned %p(%p/0x%zx) instead\n", (((uintptr_t)image)&max_align)?"Error":"Warning", (void*)(head->vaddr?head->vaddr:offs), head->memsz, head->name, image, raw, head->align);
        offs = (uintptr_t)image;
        if(((uintptr_t)image)&max_align) {
            munmap(raw, sz);
            return 1;   // that's an error, alocated memory is not aligned properly
        }
    }
    if(image==MAP_FAILED || image!=(void*)(head->vaddr?head->vaddr:offs)) {
        printf_log(LOG_NONE, "%s cannot create memory map (@%p 0x%zx) for elf \"%s\"", (image==MAP_FAILED)?"Error:":"Warning:", (void*)(head->vaddr?head->vaddr:offs), head->memsz, head->name);
        if(image==MAP_FAILED) {
            printf_log(LOG_NONE, " error=%d/%s\n", errno, strerror(errno));
        } else {
            printf_log(LOG_NONE, " got %p\n", image);
        }
        if(image==MAP_FAILED)
            return 1;
        offs = (uintptr_t)image-head->vaddr;
    }
    printf_dump(log_level, "Pre-allocated 0x%zx byte at %p for %s\n", head->memsz, image, head->name);
    head->delta = offs;
    printf_dump(log_level, "Delta of %p (vaddr=%p) for Elf \"%s\"\n", (void*)offs, (void*)head->vaddr, head->name);

    head->image = image;
    head->raw = raw;
    head->raw_size = sz;
    setProtection_elf((uintptr_t)raw, sz, 0);

    head->multiblocks = (multiblock_t*)box_calloc(head->multiblock_n, sizeof(multiblock_t));
    head->tlsbase = AddTLSPartition(context, head->tlssize);
    // and now, create all individual blocks
    head->memory = (char*)0xffffffff;
    int n = 0;
    for (size_t i=0; i<head->numPHEntries; ++i) {
        if(head->PHEntries[i].p_type == PT_LOAD && head->PHEntries[i].p_flags) {
            Elf32_Phdr * e = &head->PHEntries[i];

            head->multiblocks[n].flags = e->p_flags;
            head->multiblocks[n].offs = e->p_offset;
            head->multiblocks[n].paddr = e->p_paddr + offs;
            head->multiblocks[n].size = e->p_filesz;
            head->multiblocks[n].align = e->p_align;
            uint8_t prot = PROT_READ|PROT_WRITE|((e->p_flags & PF_X)?PROT_EXEC:0);
            // check if alignment is correct
            uintptr_t balign = head->multiblocks[n].align-1;
            if(balign<4095) balign = 4095;
            head->multiblocks[n].asize = (e->p_memsz+(e->p_paddr&balign)+4095)&~4095;
            int try_mmap = 1;
            if(e->p_paddr&balign)
                try_mmap = 0;
            if(e->p_offset&(box86_pagesize-1))
                try_mmap = 0;
            if(ALIGN(e->p_memsz)!=ALIGN(e->p_filesz))
                try_mmap = 0;
            if(!e->p_filesz)
                try_mmap = 0;
            if(e->p_align<box86_pagesize)
                try_mmap = 0;
            if(try_mmap) {
                printf_dump(log_level, "Mmaping 0x%lx(0x%lx) bytes @%p for Elf \"%s\"\n", head->multiblocks[n].size, head->multiblocks[n].asize, (void*)head->multiblocks[n].paddr, head->name);
                void* p = mmap64(
                    (void*)head->multiblocks[n].paddr, 
                    head->multiblocks[n].size, 
                    prot,
                    MAP_PRIVATE|MAP_FIXED, //((prot&PROT_WRITE)?MAP_SHARED:MAP_PRIVATE)|MAP_FIXED,
                    head->fileno,
                    e->p_offset
                );
                if(p==MAP_FAILED || p!=(void*)head->multiblocks[n].paddr) {
                    try_mmap = 0;
                    printf_dump(log_level, "Mapping failed, using regular mmap+read");
                } else {
                    if(e->p_memsz>e->p_filesz && (prot&PROT_WRITE))
                        memset((void*)((uintptr_t)p + e->p_filesz), 0, e->p_memsz-e->p_filesz);
                    setProtection_elf((uintptr_t)p, head->multiblocks[n].asize, prot);
                    head->multiblocks[n].p = p;

                }
            }
            if(!try_mmap) {
                uintptr_t paddr = head->multiblocks[n].paddr&~balign;
                size_t asize = head->multiblocks[n].asize;
                void* p = MAP_FAILED;
                if(paddr==(paddr&~(box86_pagesize-1)) && (asize==ALIGN(asize))) {
                    printf_dump(log_level, "Allocating 0x%zx (0x%zx) bytes @%p, will read 0x%zx @%p for Elf \"%s\"\n", asize, e->p_memsz, (void*)paddr, e->p_filesz, (void*)head->multiblocks[n].paddr, head->name);
                    p = mmap64(
                        (void*)paddr,
                        asize,
                        prot|PROT_WRITE,
                        MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED,
                        -1,
                        0
                    );
                } else {
                    // difference in pagesize, so need to mmap only what needed to be...
                    //check startint point
                    uintptr_t new_addr = paddr;
                    ssize_t new_size = asize;
                    while(getProtection(new_addr) && (new_size>0)) {
                        new_size -= ALIGN(new_addr) - new_addr;
                        new_addr = ALIGN(new_addr);
                    }
                    if(new_size>0) {
                        printf_dump(log_level, "Allocating 0x%zx (0x%zx) bytes @%p, will read 0x%zx @%p for Elf \"%s\"\n", ALIGN(new_size), e->p_memsz, (void*)new_addr, e->p_filesz, (void*)head->multiblocks[n].paddr, head->name);
                        p = mmap64(
                            (void*)new_addr,
                            ALIGN(new_size),
                            prot|PROT_WRITE,
                            MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED,
                            -1,
                            0
                        );
                        if(p==(void*)new_addr)
                            p = (void*)paddr;
                    } else {
                        p = (void*)paddr;
                        printf_dump(log_level, "Will read 0x%zx @%p for Elf \"%s\"\n", e->p_filesz, (void*)head->multiblocks[n].paddr, head->name);    
                    }
                }
                if(p==MAP_FAILED || p!=(void*)paddr) {
                    printf_log(LOG_NONE, "Cannot create memory map (@%p 0x%zx/0x%zx) for elf \"%s\"", (void*)paddr, asize, balign, head->name);
                    if(p==MAP_FAILED) {
                        printf_log(LOG_NONE, " error=%d/%s\n", errno, strerror(errno));
                    } else {
                        printf_log(LOG_NONE, " got %p\n", p);
                    }
                    return 1;
                }
                setProtection_elf((uintptr_t)p, asize, prot);
                head->multiblocks[n].p = p;
                if(e->p_filesz) {
                    fseeko64(head->file, head->multiblocks[n].offs, SEEK_SET);
                    if(fread((void*)head->multiblocks[n].paddr, head->multiblocks[n].size, 1, head->file)!=1) {
                        printf_log(LOG_NONE, "Cannot read elf block (@%p 0x%zx/0x%zx) for elf \"%s\"\n", (void*)head->multiblocks[n].offs, head->multiblocks[n].asize, balign, head->name);
                        return 1;
                    }
                }
                if(!(prot&PROT_WRITE) && (paddr==(paddr&(box86_pagesize-1)) && (asize==ALIGN(asize))))
                    mprotect((void*)paddr, asize, prot);
            }
#ifdef DYNAREC
            if(box86_dynarec && (e->p_flags & PF_X)) {
                dynarec_log(LOG_DEBUG, "Add ELF eXecutable Memory %p:%p\n", head->multiblocks[n].p, (void*)head->multiblocks[n].asize);
                addDBFromAddressRange((uintptr_t)head->multiblocks[n].p, head->multiblocks[n].asize);
            }
#endif
            if((uintptr_t)head->memory>(uintptr_t)head->multiblocks[n].p)
                head->memory = (char*)head->multiblocks[n].p;
            ++n;
        }
        if(head->PHEntries[i].p_type == PT_TLS) {
            Elf32_Phdr * e = &head->PHEntries[i];
            char* dest = (char*)(context->tlsdata+context->tlssize+head->tlsbase);
            printf_log(LOG_DEBUG, "Loading TLS block #%zu @%p (0x%zx/0x%zx)\n", i, dest, e->p_filesz, e->p_memsz);
            if(e->p_filesz) {
                fseeko64(head->file, e->p_offset, SEEK_SET);
                if(fread(dest, e->p_filesz, 1, head->file)!=1) {
                    printf_log(LOG_NONE, "Fail to read PT_TLS part #%zu (size=%zd)\n", i, e->p_filesz);
                    return 1;
                }
            }
            // zero'd difference between filesz and memsz
            if(e->p_filesz != e->p_memsz)
                memset(dest+e->p_filesz, 0, e->p_memsz - e->p_filesz);
        }
    }

    // can close the elf file now!
    fclose(head->file);
    head->file = NULL;
    head->fileno = -1;

    return 0;
}

void FreeElfMemory(elfheader_t* head)
{
    if(head->multiblock_n) {
#ifdef DYNAREC
        for(int i=0; i<head->multiblock_n; ++i) {
            dynarec_log(LOG_INFO, "Free DynaBlocks %p-%p for %s\n", head->multiblocks[i].p, head->multiblocks[i].p+head->multiblocks[i].asize, head->path);
            if(box86_dynarec)
                cleanDBFromAddressRange((uintptr_t)head->multiblocks[i].p, head->multiblocks[i].asize, 1);
            freeProtection((uintptr_t)head->multiblocks[i].p, head->multiblocks[i].asize);
        }
#endif
        box_free(head->multiblocks);
    }
    // we only need to free the overall mmap, no need to free individual part as they are inside the big one
    if(head->raw && head->raw_size) {
        dynarec_log(LOG_INFO, "Unmap elf memory %p-%p for %s\n", head->raw, head->raw+head->raw_size, head->path);
        munmap(head->raw, head->raw_size);
    }
    freeProtection((uintptr_t)head->raw, head->raw_size);
}

int isElfHasNeededVer(elfheader_t* head, const char* libname, elfheader_t* verneeded)
{
    if(!verneeded || !head)
        return 1;
    if(!head->VerDef || !verneeded->VerNeed)
        return 1;
    int cnt = GetNeededVersionCnt(verneeded, libname);
    for (int i=0; i<cnt; ++i) {
        const char* vername = GetNeededVersionString(verneeded, libname, i);
        if(vername && !GetVersionIndice(head, vername)) {
            printf_log(/*LOG_DEBUG*/LOG_INFO, "Discarding %s for missing version %s\n", head->path, vername);
            return 0;   // missing version
        }
    }
    return 1;
}

int FindR386COPYRel(elfheader_t* h, const char* name, uintptr_t *offs, uint32_t** p, size_t size, int version, const char* vername)
{
    if(!h)
        return 0;
    if(!h->rel)
        return 0;
    if(h->relent) {
        Elf32_Rel * rel = (Elf32_Rel *)(h->rel + h->delta);
        int cnt = h->relsz / h->relent;
        for (int i=0; i<cnt; ++i) {
            int t = ELF32_R_TYPE(rel[i].r_info);
            Elf32_Sym *sym = &h->DynSym[ELF32_R_SYM(rel[i].r_info)];
            const char* symname = SymName(h, sym);
            if((t==R_386_COPY) && symname && !strcmp(symname, name) && (sym->st_size==size)) {
                int version2 = h->VerSym?((Elf32_Half*)((uintptr_t)h->VerSym+h->delta))[ELF32_R_SYM(rel[i].r_info)]:-1;
                if(version2!=-1) version2 &= 0x7fff;
                if(version && !version2) version2=-1;   // match a versionned symbol against a global "local" symbol
                const char* vername2 = GetSymbolVersion(h, version2);
                if(SameVersionedSymbol(name, version, vername, symname, version2, vername2)) {
                    if(offs) *offs = sym->st_value + h->delta;
                    if(p) *p = (uint32_t*)(rel[i].r_offset + h->delta);
                    return 1;
                }
            }
        }
    }
    if(h->relaent) {
        int cnt = h->relasz / h->relaent;
        for (int i=0; i<cnt; ++i) {
            Elf32_Rela * rela = (Elf32_Rela *)(h->rela + h->delta);
            int t = ELF32_R_TYPE(rela[i].r_info);
            Elf32_Sym *sym = &h->DynSym[ELF32_R_SYM(rela[i].r_info)];
            const char* symname = SymName(h, sym);
            if(t==R_386_COPY && symname && !strcmp(symname, name) && sym->st_size==size) {
                int version2 = h->VerSym?((Elf32_Half*)((uintptr_t)h->VerSym+h->delta))[ELF32_R_SYM(rela[i].r_info)]:-1;
                if(version2!=-1) version2 &= 0x7fff;
                if(version && !version2) version2=-1;   // match a versionned symbol against a global "local" symbol
                const char* vername2 = GetSymbolVersion(h, version2);
                if(SameVersionedSymbol(name, version, vername, symname, version2, vername2)) {
                    *offs = sym->st_value + h->delta;
                    *p = (uint32_t*)(rela[i].r_offset + h->delta + rela[i].r_addend);
                    return 1;
                }
            }
        }
    }
    return 0;
}

int RelocateElfREL(lib_t *maplib, lib_t *local_maplib, int bindnow, elfheader_t* head, int cnt, Elf32_Rel *rel, int* need_resolv)
{
    int ret_ok = 0;
    const char* old_globdefver = NULL;
    const char* old_weakdefver = NULL;
    int old_bind = -1;
    const char* old_symname = NULL;
    uintptr_t old_offs = 0;
    uintptr_t old_end = 0;
    int old_version = -1;
    for (int i=0; i<cnt; ++i) {
        int t = ELF32_R_TYPE(rel[i].r_info);
        Elf32_Sym *sym = &head->DynSym[ELF32_R_SYM(rel[i].r_info)];
        int bind = ELF32_ST_BIND(sym->st_info);
        //uint32_t ndx = sym->st_shndx;
        const char* symname = SymName(head, sym);
        uint32_t *p = (uint32_t*)(rel[i].r_offset + head->delta);
        uintptr_t offs = 0;
        uintptr_t end = 0;
        size_t size = sym->st_size;
        elfheader_t* h_tls = NULL;//head;
        int version = head->VerSym?((Elf32_Half*)((uintptr_t)head->VerSym+head->delta))[ELF32_R_SYM(rel[i].r_info)]:-1;
        if(version!=-1) version &=0x7fff;
        const char* vername = GetSymbolVersion(head, version);
        const char* globdefver = NULL;
        const char* weakdefver = NULL;
        if(old_bind==bind && old_symname==symname) {
            globdefver = old_globdefver;
            weakdefver = old_weakdefver;
        } else {
            old_globdefver = globdefver = (bind==STB_WEAK)?NULL:GetMaplibDefaultVersion(maplib, local_maplib, 0, symname);
            old_weakdefver = weakdefver = (bind==STB_WEAK || !globdefver)?GetMaplibDefaultVersion(maplib, local_maplib, 1, symname):NULL;
        }
        if(bind==STB_LOCAL) {
            if(!symname || !symname[0]) {
                offs = sym->st_value + head->delta;
                end = offs + sym->st_size;
            } else {
                if(old_version==version && old_bind==bind && old_symname==symname) {
                    offs = old_offs;
                    end = old_end;
                } else {
                    if(local_maplib)
                        GetLocalSymbolStartEnd(local_maplib, symname, &offs, &end, head, version, vername, globdefver, weakdefver);
                    if(!offs && !end)
                        GetLocalSymbolStartEnd(maplib, symname, &offs, &end, head, version, vername, globdefver, weakdefver);
                }
            }
        } else {
            // this is probably very very wrong. A proprer way to get reloc need to be written, but this hack seems ok for now
            // at least it work for half-life, unreal, ut99, zsnes, Undertale, ColinMcRae Remake, FTL, ShovelKnight...
            /*if(bind==STB_GLOBAL && (ndx==10 || ndx==19) && t!=R_386_GLOB_DAT) {
                offs = sym->st_value + head->delta;
                end = offs + sym->st_size;
            }*/
            // so weak symbol are the one left
            if(bind==STB_WEAK) {
                if(old_version==version && old_bind==bind && old_symname==symname) {
                    offs = old_offs;
                    end = old_end;
                } else {
                    if(!offs && !end && local_maplib)
                        GetGlobalWeakSymbolStartEnd(local_maplib, symname, &offs, &end, head, version, vername, globdefver, weakdefver);
                    if(!offs && !end)
                        GetGlobalWeakSymbolStartEnd(maplib, symname, &offs, &end, head, version, vername, globdefver, weakdefver);
                }
            } else {
                if(old_version==version && old_bind==bind && old_symname==symname) {
                    offs = old_offs;
                    end = old_end;
                } else {
                    GetGlobalSymbolStartEnd(maplib, symname, &offs, &end, head, version, vername, globdefver, weakdefver);
                    if(!offs && !end && local_maplib)
                        GetGlobalSymbolStartEnd(local_maplib, symname, &offs, &end, head, version, vername, globdefver, weakdefver);
                }
            }
        }
        old_bind = bind;
        old_symname = symname;
        old_offs = offs;
        old_end = end;
        uintptr_t globoffs, globend;
        uint32_t* globp;
        uintptr_t tmp = 0;
        intptr_t delta;
        switch(t) {
            case R_386_NONE:
                // can be ignored
                printf_dump(LOG_NEVER, "Ignoring [%d] %s %p (%p)\n", i, DumpRelType(t), p, (void*)(p?(*p):0));
                break;
            case R_386_PC32:
                    if (!offs) {
                        printf_log(LOG_NONE, "Error: Global Symbol %s not found, cannot apply R_386_PC32 %p (%p) in %s\n", symname, p, *(void**)p, head->name);
                        ret_ok = 1;
                    }
                    if(offs)
                        printf_dump(LOG_NEVER, "Apply [%d] %s R_386_PC32 %p with sym=%s (ver=%d/%s), (%p -> %p/%p)\n", i, (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), p, symname, version, vername?vername:"(none)", *(void**)p, (void*)(*(uintptr_t*)p+(offs-(uintptr_t)p)), (void*)offs);
                    offs = (offs - (uintptr_t)p);
                    *p += offs;
                break;
            case R_386_RELATIVE:
                printf_dump(LOG_NEVER, "Apply [%d] %s R_386_RELATIVE %p (%p -> %p)\n", i, (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), p, *(void**)p, (void*)((*p)+head->delta));
                *p += head->delta;
                break;
            case R_386_COPY:
                globoffs = offs;
                globend = end;
                offs = end = 0;
                GetSizedSymbolStartEnd(my_context->globdata, symname, &offs, &end, size, version, vername, 1, globdefver); // try globaldata symbols first
                if(!offs && local_maplib)
                    GetNoSelfSymbolStartEnd(local_maplib, symname, &offs, &end, head, size, version, vername, globdefver, weakdefver);
                if(!offs)
                    GetNoSelfSymbolStartEnd(maplib, symname, &offs, &end, head, size, version, vername, globdefver, weakdefver);
                if(!offs) {offs = globoffs; end = globend;}
                if(offs) {
                    // add r_addend to p?
                    printf_dump(LOG_NEVER, "Apply R_386_COPY @%p with sym=%s (ver=%d/%s), @%p size=%zd\n", p, symname, version, vername?vername:"(none)", (void*)offs, sym->st_size);
                    if(p!=(void*)offs)
                        memmove(p, (void*)offs, sym->st_size);
                } else {
                    printf_log(LOG_NONE, "Error: Symbol %s not found, cannot apply RELA R_386_COPY @%p (%p) in %s\n", symname, p, *(void**)p, head->name);
                }
                break;
            case R_386_GLOB_DAT:
                if((head!=my_context->elfs[0]) && !IsGlobalNoWeakSymbolInNative(maplib, symname, version, vername, globdefver) && FindR386COPYRel(my_context->elfs[0], symname, &globoffs, &globp, size, version, vername)) {
                    // set global offs / size for the symbol
                    offs = sym->st_value + head->delta;
                    end = offs + sym->st_size;
                    if(sym->st_size && offs) {
                        printf_dump(LOG_NEVER, "Apply %s R_386_GLOB_DAT with R_386_COPY @%p/%p (%p/%p -> %p/%p) size=%zd on sym=%s (ver=%d/%s) \n", 
                            (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), p, globp, (void*)(p?(*p):0), 
                            (void*)(globp?(*globp):0), (void*)offs, (void*)globoffs, sym->st_size, symname, version, vername?vername:"(none)");
                        //memmove((void*)globoffs, (void*)offs, sym->st_size);   // preapply to copy part from lib to main elf
                        AddUniqueSymbol(GetGlobalData(maplib), symname, globoffs, sym->st_size, version, vername);
                        AddUniqueSymbol(my_context->globdata, symname, offs, sym->st_size, version, vername);
                    } else {
                        printf_dump(LOG_NEVER, "Apply %s R_386_GLOB_DAT with R_386_COPY @%p/%p (%p/%p -> %p/%p) null sized on sym=%s (ver=%d/%s)\n", 
                            (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), p, globp, (void*)(p?(*p):0), 
                            (void*)(globp?(*globp):0), (void*)offs, (void*)globoffs, symname, version, vername?vername:"(none)");
                    }
                    *p = globoffs;
                } else {
                    if((size==0 )&& GetSymbolStartEnd(GetGlobalData(maplib), symname, &globoffs, &globend, version, vername, 0, globdefver)) {
                        offs = globoffs;
                    }
                    if (!offs) {
                        if(strcmp(symname, "__gmon_start__") && strcmp(symname, "data_start") && strcmp(symname, "__data_start")) {
                            printf_log((bind==STB_GLOBAL)?LOG_NONE:LOG_INFO, "%s: Global Symbol %s not found, cannot apply R_386_GLOB_DAT @%p (%p) in %s\n", (bind==STB_GLOBAL)?"Error":"Warning", symname, p, *(void**)p, head->name);
                            if(bind==STB_GLOBAL) ret_ok=1;
                        }
                    } else {
                        printf_dump(LOG_NEVER, "Apply %s R_386_GLOB_DAT @%p (%p -> %p) on sym=%s (ver=%d/%s)\n", (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), p, (void*)(p?(*p):0), (void*)offs, symname, version, vername?vername:"(none)");
                        *p = offs/* + rela[i].r_addend*/;   // not addend it seems
                    }
                }
                break;
            case R_386_JMP_SLOT:
                // apply immediatly for gobject closure marshal or for LOCAL binding. Also, apply immediatly if it doesn't jump in the got
                tmp = (uintptr_t)(*p);
                if (bind==STB_LOCAL 
                  || ((symname && strstr(symname, "g_cclosure_marshal_")==symname)) 
                  || ((symname && strstr(symname, "__pthread_unwind_next")==symname)) 
                  || !tmp
                  || !((tmp>=head->plt && tmp<head->plt_end) || (tmp>=head->gotplt && tmp<head->gotplt_end))
                  || !need_resolv
                  || bindnow
                  ) {
                    if (!offs) {
                        if(bind==STB_WEAK) {
                            printf_log(LOG_INFO, "Warning: Weak Symbol %s not found, cannot apply R_386_JMP_SLOT %p (%p)\n", symname, p, *(void**)p);
                        } else {
                            printf_log(LOG_NONE, "Error: Symbol %s not found, cannot apply R_386_JMP_SLOT %p (%p) in %s\n", symname, p, *(void**)p, head->name);
                            ret_ok = 1;
                        }
                    } else {
                        if(p) {
                            printf_dump(LOG_NEVER, "Apply %s R_386_JMP_SLOT %p with sym=%s(%s%s%s) (%p -> %p)\n", (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), p, symname, symname, vername?"@":"", vername?vername:"", *(void**)p, (void*)offs);
                            *p = offs;
                        } else {
                            printf_log(LOG_NONE, "Warning, Symbol %s found, but Jump Slot Offset is NULL \n", symname);
                        }
                    }
                } else {
                    printf_dump(LOG_NEVER, "Preparing (if needed) %s R_386_JMP_SLOT %p (0x%x->0x%0x) with sym=%s(%s%s%s/version %d) to be apply later\n", (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), p, *p, *p+head->delta, symname, symname, vername?"@":"", vername?vername:"", version);
                    *p += head->delta;
                    *need_resolv = 1;
                }
                break;
            case R_386_32:
                if (!offs) {
                        if(strcmp(symname, "__gmon_start__") && strcmp(symname, "data_start") && strcmp(symname, "__data_start")) {
                            printf_log(LOG_NONE, "Error: Symbol sym=%s(%s%s%s/version %d) not found, cannot apply R_386_32 %p (%p) in %s\n", symname, symname, vername?"@":"", vername?vername:"", version, p, *(void**)p, head->name);
                            ret_ok = 1;
                        }
                } else {
                    printf_dump(LOG_NEVER, "Apply %s R_386_32 %p with sym=%s (ver=%d/%s) (%p -> %p)\n", (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), p, symname, version, vername?vername:"(none)", *(void**)p, (void*)(offs+*(uint32_t*)p));
                    *p += offs;
                }
                break;
            case R_386_TLS_TPOFF:
                // Negated offset in static TLS block
                {
                    if(!symname || !symname[0] || bind==STB_LOCAL) {
                        h_tls = head;
                        offs = sym->st_value;
                    } else {
                        h_tls = NULL;
                        if(local_maplib)
                            h_tls = GetGlobalSymbolElf(local_maplib, symname, version, vername);
                        if(!h_tls)
                            h_tls = GetGlobalSymbolElf(maplib, symname, version, vername);
                    }
                    if(h_tls) {
                        delta = *(int32_t*)p;
                        printf_dump(LOG_NEVER, "Applying %s %s on %s @%p (%d -> %d+%d, size=%d)\n", (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), DumpRelType(t), symname, p, delta, h_tls->tlsbase, (int32_t)offs, end-offs);
                        *p = (uintptr_t)((int32_t)offs + h_tls->tlsbase);
                    } else {
                        printf_log(LOG_INFO, "Warning, cannot apply %s %s on %s @%p (%d), no elf_header found\n", (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), DumpRelType(t), symname, p, (int32_t)offs);
                    }
                }
                break;
            case R_386_TLS_TPOFF32:
                // Non-negated offset in static TLS block???
                {
                    if(!symname || !symname[0]) {
                        h_tls = head;
                        offs = sym->st_value;
                    } else {
                        h_tls = NULL;
                        if(local_maplib)
                            h_tls = GetGlobalSymbolElf(local_maplib, symname, version, vername);
                        if(!h_tls)
                            h_tls = GetGlobalSymbolElf(maplib, symname, version, vername);
                    }
                    if(h_tls) {
                        delta = *(int32_t*)p;
                        printf_dump(LOG_NEVER, "Applying %s %s on %s @%p (%d -> %d+%d, size=%d)\n", (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), DumpRelType(t), symname, p, delta, -h_tls->tlsbase, (int32_t)offs, end-offs);
                        *p = (uintptr_t)((int32_t)offs - h_tls->tlsbase);
                    } else {
                        printf_log(LOG_INFO, "Warning, cannot apply %s %s on %s @%p (%d), no elf_header found\n", (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), DumpRelType(t), symname, p, (int32_t)offs);
                    }
                }
                break;
            case R_386_TLS_DTPMOD32:
                // ID of module containing symbol
                if(!symname || symname[0]=='\0' || bind==STB_LOCAL)
                    offs = getElfIndex(my_context, head);
                else {
                    if(!h_tls) {
                        if(local_maplib)
                            h_tls = GetGlobalSymbolElf(local_maplib, symname, version, vername);
                        if(!h_tls)
                            h_tls = GetGlobalSymbolElf(maplib, symname, version, vername);
                    }
                    offs = getElfIndex(my_context, h_tls);
                }
                if(p) {
                    printf_dump(LOG_NEVER, "Apply %s %s %p with sym=%s (%p -> %p)\n", "R_386_TLS_DTPMOD32", (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), p, symname, *(void**)p, (void*)offs);
                    *p = offs;
                } else {
                    printf_log(LOG_NONE, "Warning, Symbol %s or Elf not found, but R_386_TLS_DTPMOD32 Slot Offset is NULL \n", symname);
                }
                break;
            case R_386_TLS_DTPOFF32:
                // Offset in TLS block
                if (!offs && !end) {
                    if(bind==STB_WEAK) {
                        printf_log(LOG_INFO, "Warning: Weak Symbol %s not found, cannot apply R_386_TLS_DTPOFF32 %p (%p)\n", symname, p, *(void**)p);
                    } else {
                        printf_log(LOG_NONE, "Error: Symbol %s not found, cannot apply R_386_TLS_DTPOFF32 %p (%p) in %s\n", symname, p, *(void**)p, head->name);
                    }
                } else {
                    if(!symname || !symname[0]) {
                        offs = (uintptr_t)((intptr_t)(head->tlsaddr + head->delta) - (intptr_t)offs);    // negative offset
                    }
                    if(p) {
                        printf_dump(LOG_NEVER, "Apply %s R_386_TLS_DTPOFF32 %p with sym=%s (ver=%d/%s) (%zd -> %zd)\n", (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), p, symname, version, vername?vername:"(none)", (intptr_t)*p, (intptr_t)offs);
                        *p = offs;
                    } else {
                        printf_log(LOG_NONE, "Warning, Symbol %s found, but R_386_TLS_DTPOFF32 Slot Offset is NULL \n", symname);
                    }
                }
                break;
            default:
                printf_log(LOG_INFO, "Warning, don't know of to handle rel #%d %s (%p) for %s\n", i, DumpRelType(ELF32_R_TYPE(rel[i].r_info)), p, symname?symname:"(nil)");
        }
    }
    return bindnow?ret_ok:0;
}

int RelocateElfRELA(lib_t *maplib, lib_t *local_maplib, int bindnow, elfheader_t* head, int cnt, Elf32_Rela *rela, int* need_resolv)
{
    int ret_ok = 0;
    const char* old_globdefver = NULL;
    const char* old_weakdefver = NULL;
    int old_bind = -1;
    const char* old_symname = NULL;
    uintptr_t old_offs = 0;
    uintptr_t old_end = 0;
    int old_version = -1;
    for (int i=0; i<cnt; ++i) {
        int t = ELF32_R_TYPE(rela[i].r_info);
        Elf32_Sym *sym = &head->DynSym[ELF32_R_SYM(rela[i].r_info)];
        int bind = ELF32_ST_BIND(sym->st_info);
        //uint32_t ndx = sym->st_shndx;
        const char* symname = SymName(head, sym);
        uint32_t *p = (uint32_t*)(rela[i].r_offset + head->delta);
        uintptr_t offs = 0;
        uintptr_t end = 0;
        size_t size = sym->st_size;
        elfheader_t* h_tls = NULL;//head;
        int version = head->VerSym?((Elf32_Half*)((uintptr_t)head->VerSym+head->delta))[ELF32_R_SYM(rela[i].r_info)]:-1;
        if(version!=-1) version &=0x7fff;
        const char* vername = GetSymbolVersion(head, version);
        const char* globdefver = NULL;
        const char* weakdefver = NULL;
        if(old_bind==bind && old_symname==symname) {
            globdefver = old_globdefver;
            weakdefver = old_weakdefver;
        } else {
            old_globdefver = globdefver = (bind==STB_WEAK)?NULL:GetMaplibDefaultVersion(maplib, local_maplib, 0, symname);
            old_weakdefver = weakdefver = (bind==STB_WEAK || !globdefver)?GetMaplibDefaultVersion(maplib, local_maplib, 1, symname):NULL;
        }
        if(bind==STB_LOCAL) {
            if(!symname || !symname[0]) {
                offs = sym->st_value + head->delta;
                end = offs + sym->st_size;
            } else {
                if(old_version==version && old_bind==bind && old_symname==symname) {
                    offs = old_offs;
                    end = old_end;
                } else {
                    if(local_maplib)
                        GetLocalSymbolStartEnd(local_maplib, symname, &offs, &end, head, version, vername, globdefver, weakdefver);
                    if(!offs && !end)
                        GetLocalSymbolStartEnd(maplib, symname, &offs, &end, head, version, vername, globdefver, weakdefver);
                }
            }
        } else {
            // this is probably very very wrong. A proprer way to get reloc need to be writen, but this hack seems ok for now
            // at least it work for half-life, unreal, ut99, zsnes, Undertale, ColinMcRae Remake, FTL, ShovelKnight...
            /*if(bind==STB_GLOBAL && (ndx==10 || ndx==19) && t!=R_386_GLOB_DAT) {
                offs = sym->st_value + head->delta;
                end = offs + sym->st_size;
            }*/
            // so weak symbol are the one left
            if(bind==STB_WEAK) {
                if(old_version==version && old_bind==bind && old_symname==symname) {
                    offs = old_offs;
                    end = old_end;
                } else {
                    GetGlobalWeakSymbolStartEnd(maplib, symname, &offs, &end, head, version, vername, globdefver, weakdefver);
                    if(!offs && !end && local_maplib)
                        GetGlobalWeakSymbolStartEnd(local_maplib, symname, &offs, &end, head, version, vername, globdefver, weakdefver);
                }
            } else {
                if(old_version==version && old_bind==bind && old_symname==symname) {
                    offs = old_offs;
                    end = old_end;
                } else {
                    if(!offs && !end && local_maplib)
                        GetGlobalSymbolStartEnd(local_maplib, symname, &offs, &end, head, version, vername, globdefver, weakdefver);
                    if(!offs && !end)
                        GetGlobalSymbolStartEnd(maplib, symname, &offs, &end, head, version, vername, globdefver, weakdefver);
                }
            }
        }
        old_bind = bind;
        old_symname = symname;
        old_offs = offs;
        old_end = end;
        uintptr_t globoffs, globend;
        uint32_t* globp;
        uintptr_t tmp = 0;
        intptr_t delta;
        switch(t) {
            case R_386_NONE:
                // can be ignored
                printf_dump(LOG_NEVER, "Ignoring [%d] %s %p (%p)\n", i, DumpRelType(t), p, (void*)(p?(*p):0));
                break;
            case R_386_PC32:
                    if (!offs) {
                        printf_log(LOG_NONE, "Error: Global Symbol %s not found, cannot apply R_386_PC32 %p (%p) in %s\n", symname, p, *(void**)p, head->name);
                    }
                    if(offs)
                        printf_dump(LOG_NEVER, "Apply [%d] %s R_386_PC32 %p with sym=%s (%p -> %p/%p)\n", i, (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), p, symname, *(void**)p, (void*)(*(uintptr_t*)p+(offs-(uintptr_t)p)), (void*)offs);
                    offs = (offs - (uintptr_t)p);
                    *p += offs;
                break;
            case R_386_RELATIVE:
                printf_dump(LOG_NEVER, "Apply [%d] %s R_386_RELATIVE %p (%p -> %p)\n", i, (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), p, *(void**)p, (void*)((*p)+head->delta));
                *p = head->delta+ rela[i].r_addend;
                break;
            case R_386_COPY:
                globoffs = offs;
                globend = end;
                offs = end = 0;
                GetSizedSymbolStartEnd(my_context->globdata, symname, &offs, &end, size, version, vername, 1, globdefver); // try globaldata symbols first
                if(!offs && local_maplib)
                    GetNoSelfSymbolStartEnd(local_maplib, symname, &offs, &end, head, size, version, vername, globdefver, weakdefver);
                if(!offs)
                    GetNoSelfSymbolStartEnd(maplib, symname, &offs, &end, head, size, version, vername, globdefver, weakdefver);
                if(!offs) {offs = globoffs; end = globend;}
                if(offs) {
                    // add r_addend to p?
                    printf_dump(LOG_NEVER, "Apply R_386_COPY @%p with sym=%s (ver=%d/%s), @%p size=%zd\n", p, symname, version, vername?vername:"(none)", (void*)offs, sym->st_size);
                    if(p!=(void*)(offs+rela[i].r_addend))
                        memmove(p, (void*)(offs+rela[i].r_addend), sym->st_size);
                } else {
                    printf_log(LOG_NONE, "Error: Symbol %s not found, cannot apply RELA R_386_COPY @%p (%p) in %s\n", symname, p, *(void**)p, head->name);
                }
                break;
            case R_386_GLOB_DAT:
                if(head!=my_context->elfs[0] && !IsGlobalNoWeakSymbolInNative(maplib, symname, version, vername, globdefver) && FindR386COPYRel(my_context->elfs[0], symname, &globoffs, &globp, size, version, vername)) {
                    // set global offs / size for the symbol
                    offs = sym->st_value + head->delta;
                    end = offs + sym->st_size;
                    if(sym->st_size && offs) {
                        printf_dump(LOG_NEVER, "Apply %s R_386_GLOB_DAT with R_386_COPY @%p/%p (%p/%p -> %p/%p) size=%zd on sym=%s (ver=%d/%s) \n", 
                            (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), p, globp, (void*)(p?(*p):0), 
                            (void*)(globp?(*globp):0), (void*)offs, (void*)globoffs, sym->st_size, symname, version, vername?vername:"(none)");
                        //memmove((void*)globoffs, (void*)offs, sym->st_size);   // preapply to copy part from lib to main elf
                        AddUniqueSymbol(GetGlobalData(maplib), symname, globoffs, sym->st_size, version, vername);
                        AddUniqueSymbol(my_context->globdata, symname, offs, sym->st_size, version, vername);
                    } else {
                        printf_dump(LOG_NEVER, "Apply %s R_386_GLOB_DAT with R_386_COPY @%p/%p (%p/%p -> %p/%p) null sized on sym=%s (ver=%d/%s)\n", 
                            (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), p, globp, (void*)(p?(*p):0), 
                            (void*)(globp?(*globp):0), (void*)offs, (void*)globoffs, symname, version, vername?vername:"(none)");
                    }
                    *p = globoffs;
                } else {
                    if((size==0 )&& GetSymbolStartEnd(GetGlobalData(maplib), symname, &globoffs, &globend, version, vername, 0, globdefver)) {
                        offs = globoffs;
                    }
                    if (!offs) {
                        if(strcmp(symname, "__gmon_start__") && strcmp(symname, "data_start") && strcmp(symname, "__data_start"))
                            printf_log(LOG_NONE, "Error: Global Symbol %s not found, cannot apply R_386_GLOB_DAT @%p (%p) in %s\n", symname, p, *(void**)p, head->name);
                    } else {
                        printf_dump(LOG_NEVER, "Apply %s R_386_GLOB_DAT @%p (%p -> %p) on sym=%s (ver=%d/%s)\n", (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), p, (void*)(p?(*p):0), (void*)offs, symname, version, vername?vername:"(none)");
                        *p = offs/* + rela[i].r_addend*/;   // not addend it seems
                    }
                }
                break;
            case R_386_JMP_SLOT:
                // apply immediatly for gobject closure marshal or for LOCAL binding. Also, apply immediatly if it doesn't jump in the got
                tmp = (uintptr_t)(*p);
                if (bind==STB_LOCAL 
                  || ((symname && strstr(symname, "g_cclosure_marshal_")==symname)) 
                  || ((symname && strstr(symname, "__pthread_unwind_next")==symname)) 
                  || !tmp
                  || !((tmp>=head->plt && tmp<head->plt_end) || (tmp>=head->gotplt && tmp<head->gotplt_end))
                  || !need_resolv
                  || bindnow
                  ) {
                    if (!offs) {
                        if(bind==STB_WEAK) {
                            printf_log(LOG_INFO, "Warning: Weak Symbol %s not found, cannot apply R_386_JMP_SLOT %p (%p)\n", symname, p, *(void**)p);
                        } else {
                            printf_log(LOG_NONE, "Error: Symbol %s not found, cannot apply R_386_JMP_SLOT %p (%p) in %s\n", symname, p, *(void**)p, head->name);
                            ret_ok = 1;
                        }
                    } else {
                        if(p) {
                            printf_dump(LOG_NEVER, "Apply %s R_386_JMP_SLOT %p with sym=%s(%s%s%s) (%p -> %p)\n", (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), p, symname, symname, vername?"@":"", vername?vername:"", *(void**)p, (void*)offs);
                            *p = offs + rela[i].r_addend;
                        } else {
                            printf_log(LOG_NONE, "Warning, Symbol %s found, but Jump Slot Offset is NULL \n", symname);
                        }
                    }
                } else {
                    printf_dump(LOG_NEVER, "Preparing (if needed) %s R_386_JMP_SLOT %p (0x%x->0x%0x) with sym=%s(%s%s%s/version %d) to be apply later\n", (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), p, *p, *p+head->delta, symname, symname, vername?"@":"", vername?vername:"", version);
                    *p += head->delta;
                    *need_resolv = 1;
                }
                break;
            case R_386_32:
                if (!offs) {
                    printf_log(LOG_NONE, "Error: Symbol %s not found, cannot apply R_386_32 %p (%p) in %s\n", symname, p, *(void**)p, head->name);
                    ret_ok = 1;
                } else {
                    printf_dump(LOG_NEVER, "Apply %s R_386_32 %p with sym=%s (ver=%d/%s) (%p -> %p)\n", (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), p, symname, version, vername?vername:"(none)", *(void**)p, (void*)(offs+*(uint32_t*)p));
                    *p /*+*/= offs+rela[i].r_addend;
                }
                break;
            case R_386_TLS_TPOFF:
                // Negated offset in static TLS block
                {
                    if(!symname || !symname[0] || bind==STB_LOCAL) {
                        h_tls = head;
                        offs = sym->st_value;
                    } else {
                        h_tls = NULL;
                        if(local_maplib)
                            h_tls = GetGlobalSymbolElf(local_maplib, symname, version, vername);
                        if(!h_tls)
                            h_tls = GetGlobalSymbolElf(maplib, symname, version, vername);
                    }
                    if(h_tls) {
                        delta = *(int32_t*)p;
                        printf_dump(LOG_NEVER, "Applying %s %s on %s @%p (%d -> %d+%d, size=%d)\n", (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), DumpRelType(t), symname, p, delta, h_tls->tlsbase, (int32_t)offs, end-offs);
                        *p = (uintptr_t)((int32_t)offs + rela[i].r_addend + h_tls->tlsbase);
                    } else {
                        printf_log(LOG_INFO, "Warning, cannot apply %s %s on %s @%p (%d), no elf_header found\n", (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), DumpRelType(t), symname, p, (int32_t)offs);
                    }
                }
                break;
            case R_386_TLS_TPOFF32:
                // Non-negated offset in static TLS block???
                {
                    if(!symname || !symname[0]) {
                        h_tls = head;
                        offs = sym->st_value;
                    } else {
                        h_tls = NULL;
                        if(local_maplib)
                            h_tls = GetGlobalSymbolElf(local_maplib, symname, version, vername);
                        if(!h_tls)
                            h_tls = GetGlobalSymbolElf(maplib, symname, version, vername);
                    }
                    if(h_tls) {
                        delta = *(int32_t*)p;
                        printf_dump(LOG_NEVER, "Applying %s %s on %s @%p (%d -> %d+%d, size=%d)\n", (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), DumpRelType(t), symname, p, delta, -h_tls->tlsbase, (int32_t)offs, end-offs);
                        *p = (uintptr_t)((int32_t)offs + rela[i].r_addend - h_tls->tlsbase);
                    } else {
                        printf_log(LOG_INFO, "Warning, cannot apply %s %s on %s @%p (%d), no elf_header found\n", (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), DumpRelType(t), symname, p, (int32_t)offs);
                    }
                }
                break;
            case R_386_TLS_DTPMOD32:
                // ID of module containing symbol
                if(!symname || symname[0]=='\0' || bind==STB_LOCAL)
                    offs = getElfIndex(my_context, head);
                else {
                    if(!h_tls) {
                        if(local_maplib)
                            h_tls = GetGlobalSymbolElf(local_maplib, symname, version, vername);
                        if(!h_tls)
                            h_tls = GetGlobalSymbolElf(maplib, symname, version, vername);
                    }
                    offs = getElfIndex(my_context, h_tls);
                }
                if(p) {
                    printf_dump(LOG_NEVER, "Apply %s %s %p with sym=%s (%p -> %p)\n", "R_386_TLS_DTPMOD32", (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), p, symname, *(void**)p, (void*)offs);
                    *p = offs;
                } else {
                    printf_log(LOG_NONE, "Warning, Symbol %s or Elf not found, but R_386_TLS_DTPMOD32 Slot Offset is NULL \n", symname);
                }
                break;
            case R_386_TLS_DTPOFF32:
                // Offset in TLS block
                if (!offs && !end) {
                    if(bind==STB_WEAK) {
                        printf_log(LOG_INFO, "Warning: Weak Symbol %s not found, cannot apply R_386_TLS_DTPOFF32 %p (%p)\n", symname, p, *(void**)p);
                    } else {
                        printf_log(LOG_NONE, "Error: Symbol %s not found, cannot apply R_386_TLS_DTPOFF32 %p (%p) in %s\n", symname, p, *(void**)p, head->name);
                    }
                } else {
                   if(!symname || !symname[0]) {
                        offs = (uintptr_t)((intptr_t)(head->tlsaddr + head->delta) - (intptr_t)offs);    // negative offset
                    }
                    if(p) {
                        int tlsoffset = offs;    // it's not an offset in elf memory
                        printf_dump(LOG_NEVER, "Apply %s R_386_TLS_DTPOFF32 %p with sym=%s (%p -> %p)\n", (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), p, symname, (void*)tlsoffset, (void*)offs);
                        *p = tlsoffset;
                    } else {
                        printf_log(LOG_NONE, "Warning, Symbol %s found, but R_386_TLS_DTPOFF32 Slot Offset is NULL \n", symname);
                    }
                }
                break;
            default:
                printf_log(LOG_INFO, "Warning, don't know of to handle rela #%d %s on %s\n", i, DumpRelType(ELF32_R_TYPE(rela[i].r_info)), symname);
        }
    }
    return bindnow?ret_ok:0;
}

int RelocateElf(lib_t *maplib, lib_t *local_maplib, int bindnow, elfheader_t* head)
{
    if(0 && (head->flags&DF_BIND_NOW) && !bindnow) { // disable for now, needs more symbol in a fow libs like gtk and nss3
        bindnow = 1;
        printf_log(LOG_DEBUG, "Forcing %s to Bind Now\n", head->name);
    }
    if(head->rel) {
        int cnt = head->relsz / head->relent;
        DumpRelTable(head, cnt, (Elf32_Rel *)(head->rel + head->delta), "Rel");
        printf_log(LOG_DEBUG, "Applying %d Relocation(s) for %s\n", cnt, head->name);
        if(RelocateElfREL(maplib, local_maplib, bindnow, head, cnt, (Elf32_Rel *)(head->rel + head->delta), NULL))
            return -1;
    }
    if(head->rela) {
        int cnt = head->relasz / head->relaent;
        DumpRelATable(head, cnt, (Elf32_Rela *)(head->rela + head->delta), "RelA");
        printf_log(LOG_DEBUG, "Applying %d Relocation(s) with Addend for %s\n", cnt, head->name);
        if(RelocateElfRELA(maplib, local_maplib, bindnow, head, cnt, (Elf32_Rela *)(head->rela + head->delta), NULL))
            return -1;
    }
    return 0;
}

int RelocateElfPlt(lib_t *maplib, lib_t *local_maplib, int bindnow, elfheader_t* head)
{
    int need_resolver = 0;
    if(0 && (head->flags&DF_BIND_NOW) && !bindnow) { // disable for now, needs more symbol in a fow libs like gtk and nss3
        bindnow = 1;
        printf_log(LOG_DEBUG, "Forcing %s to Bind Now\n", head->name);
    }
    if(head->pltrel) {
        int cnt = head->pltsz / head->pltent;
        if(head->pltrel==DT_REL) {
            DumpRelTable(head, cnt, (Elf32_Rel *)(head->jmprel + head->delta), "PLT");
            printf_log(LOG_DEBUG, "Applying %d PLT Relocation(s) for %s\n", cnt, head->name);
            if(RelocateElfREL(maplib, local_maplib, bindnow, head, cnt, (Elf32_Rel *)(head->jmprel + head->delta), &need_resolver))
                return -1;
        } else if(head->pltrel==DT_RELA) {
            DumpRelATable(head, cnt, (Elf32_Rela *)(head->jmprel + head->delta), "PLT");
            printf_log(LOG_DEBUG, "Applying %d PLT Relocation(s) with Addend for %s\n", cnt, head->name);
            if(RelocateElfRELA(maplib, local_maplib, bindnow, head, cnt, (Elf32_Rela *)(head->jmprel + head->delta), &need_resolver))
                return -1;
        }
        if(need_resolver) {
            if(pltResolver==~(uintptr_t)0) {
                pltResolver = AddBridge(my_context->system, vFEv, PltResolver, 0, "(PltResolver)");
            }
            if(head->pltgot) {
                *(uintptr_t*)(head->pltgot+head->delta+8) = pltResolver;
                *(uintptr_t*)(head->pltgot+head->delta+4) = (uintptr_t)head;
                printf_log(LOG_DEBUG, "PLT Resolver injected in plt.got at %p\n", (void*)(head->pltgot+head->delta+8));
            } else if(head->got) {
                *(uintptr_t*)(head->got+head->delta+8) = pltResolver;
                *(uintptr_t*)(head->got+head->delta+4) = (uintptr_t)head;
                printf_log(LOG_DEBUG, "PLT Resolver injected in got at %p\n", (void*)(head->got+head->delta+8));
            }
        }
    }
   
    return 0;
}

void CalcStack(elfheader_t* elf, uint32_t* stacksz, size_t* stackalign)
{
    if(*stacksz < elf->stacksz)
        *stacksz = elf->stacksz;
    if(*stackalign < elf->stackalign)
        *stackalign = elf->stackalign;
}

Elf32_Sym* GetFunction(elfheader_t* h, const char* name)
{
    // TODO: create a hash on named to avoid this loop
    for (uint32_t i=0; i<h->numSymTab; ++i) {
        int type = ELF32_ST_TYPE(h->SymTab[i].st_info);
        if(type==STT_FUNC) {
            const char * symname = h->StrTab+h->SymTab[i].st_name;
            if(strcmp(symname, name)==0) {
                return h->SymTab+i;
            }
        }
    }
    return NULL;
}

Elf32_Sym* GetElfObject(elfheader_t* h, const char* name)
{
    for (uint32_t i=0; i<h->numSymTab; ++i) {
        int type = ELF32_ST_TYPE(h->SymTab[i].st_info);
        if(type==STT_OBJECT) {
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
    (void)maplib;
    uintptr_t ep = h->entrypoint + h->delta;
    printf_log(LOG_DEBUG, "Entry Point is %p\n", (void*)ep);
    if(box86_dump) {
        printf_dump(LOG_NEVER, "(short) Dump of Entry point\n");
        int sz = 64;
        uintptr_t lastbyte = GetLastByte(h);
        if (ep + sz >  lastbyte)
            sz = lastbyte - ep;
        DumpBinary((char*)ep, sz);
    }
    return ep;
}

uintptr_t GetLastByte(elfheader_t* h)
{
    return (uintptr_t)h->memory/* + h->delta*/ + h->memsz;
}

#ifndef STB_GNU_UNIQUE
#define STB_GNU_UNIQUE 10
#endif

void checkHookedSymbols(elfheader_t* h); // in mallochook.c
void AddSymbols(lib_t *maplib, kh_mapsymbols_t* mapsymbols, kh_mapsymbols_t* weaksymbols, kh_mapsymbols_t* localsymbols, elfheader_t* h)
{
    if(box86_dump && h->DynSym) DumpDynSym(h);
    printf_dump(LOG_NEVER, "Will look for Symbol to add in SymTable(%zu)\n", h->numSymTab);
    for (size_t i=0; i<h->numSymTab; ++i) {
        const char * symname = h->StrTab+h->SymTab[i].st_name;
        int bind = ELF32_ST_BIND(h->SymTab[i].st_info);
        int type = ELF32_ST_TYPE(h->SymTab[i].st_info);
        int vis = h->SymTab[i].st_other&0x3;
        size_t sz = h->SymTab[i].st_size;
        if((type==STT_OBJECT || type==STT_FUNC || type==STT_COMMON || type==STT_TLS  || type==STT_NOTYPE) 
        && (vis==STV_DEFAULT || vis==STV_PROTECTED || (vis==STV_HIDDEN && bind==STB_LOCAL)) && (h->SymTab[i].st_shndx!=0)) {
            if(sz && strstr(symname, "@@")) {
                char symnameversioned[strlen(symname)+1];
                strcpy(symnameversioned, symname);
                // extact symname@@vername
                char* p = strchr(symnameversioned, '@');
                *p=0;
                p+=2;
                symname = AddDictionnary(my_context->versym, symnameversioned);
                const char* vername = AddDictionnary(my_context->versym, p);
                AddDefaultVersion((bind==STB_WEAK)?h->weakdefver:h->globaldefver, symname, vername);
                if((bind==STB_GNU_UNIQUE /*|| (bind==STB_GLOBAL && type==STT_FUNC)*/) && FindGlobalSymbol(maplib, symname, 2, p))
                    continue;
                uintptr_t offs = (type==STT_TLS)?h->SymTab[i].st_value:(h->SymTab[i].st_value + h->delta);
                printf_dump(LOG_NEVER, "Adding Default Versioned Symbol(bind=%s) \"%s@%s\" with offset=%p sz=%zu\n", (bind==STB_LOCAL)?"LOCAL":((bind==STB_WEAK)?"WEAK":"GLOBAL"), symname, vername, (void*)offs, sz);
                if(bind==STB_LOCAL)
                    AddSymbol(localsymbols, symname, offs, sz, 2, vername);
                else    // add in local and global map 
                    if(bind==STB_WEAK) {
                        AddSymbol(weaksymbols, symname, offs, sz, 2, vername);
                    } else {
                        AddSymbol(mapsymbols, symname, offs, sz, 2, vername);
                    }
            } else {
                int to_add = 1;
                if(!to_add || (bind==STB_GNU_UNIQUE && FindGlobalSymbol(maplib, symname, -1, NULL)))
                    continue;
                uintptr_t offs = (type==STT_TLS)?h->SymTab[i].st_value:(h->SymTab[i].st_value + h->delta);
                printf_dump(LOG_NEVER, "Adding Symbol(bind=%s) \"%s\" with offset=%p sz=%zu\n", (bind==STB_LOCAL)?"LOCAL":((bind==STB_WEAK)?"WEAK":"GLOBAL"), symname, (void*)offs, sz);
                if(bind==STB_LOCAL)
                    AddSymbol(localsymbols, symname, offs, sz, 1, NULL);
                else    // add in local and global map 
                    if(bind==STB_WEAK) {
                        AddSymbol(weaksymbols, symname, offs, sz, 1, NULL);
                    } else {
                        AddSymbol(mapsymbols, symname, offs, sz, 1, NULL);
                    }
            }
        }
    }
    
    printf_dump(LOG_NEVER, "Will look for Symbol to add in DynSym (%zu)\n", h->numDynSym);
    for (size_t i=0; i<h->numDynSym; ++i) {
        const char * symname = h->DynStr+h->DynSym[i].st_name;
        int bind = ELF32_ST_BIND(h->DynSym[i].st_info);
        int type = ELF32_ST_TYPE(h->DynSym[i].st_info);
        int vis = h->DynSym[i].st_other&0x3;
        if((type==STT_OBJECT || type==STT_FUNC || type==STT_COMMON || type==STT_TLS  || type==STT_NOTYPE) 
        && (vis==STV_DEFAULT || vis==STV_PROTECTED || (vis==STV_HIDDEN && bind==STB_LOCAL)) && (h->DynSym[i].st_shndx!=0 && h->DynSym[i].st_shndx<=65521)) {
            uintptr_t offs = (type==STT_TLS)?h->DynSym[i].st_value:(h->DynSym[i].st_value + h->delta);
            size_t sz = h->DynSym[i].st_size;
            int version = h->VerSym?((Elf32_Half*)((uintptr_t)h->VerSym+h->delta))[i]:-1;
            int add_default = (version!=-1 && (version&0x7fff)>1 && !(version&0x8000) && !GetMaplibDefaultVersion(my_context->maplib, (maplib==my_context->maplib)?NULL:maplib, (bind==STB_WEAK)?1:0, symname))?1:0;
            if(version!=-1) version &= 0x7fff;
            const char* vername = GetSymbolVersion(h, version);
            if(add_default) {
                AddDefaultVersion((bind==STB_WEAK)?h->weakdefver:h->globaldefver, symname, vername);
                printf_dump(LOG_NEVER, "Adding Default Version \"%s\" for Symbol\"%s\"\n", vername, symname);
            }
            int to_add = 1;
            if(!to_add || (bind==STB_GNU_UNIQUE && FindGlobalSymbol(maplib, symname, version, vername)))
                continue;
            printf_dump(LOG_NEVER, "Adding Versioned Symbol(bind=%s) \"%s\" (ver=%d/%s) with offset=%p sz=%zu\n", (bind==STB_LOCAL)?"LOCAL":((bind==STB_WEAK)?"WEAK":"GLOBAL"), symname, version, vername?vername:"(none)", (void*)offs, sz);
            if(bind==STB_LOCAL)
                AddSymbol(localsymbols, symname, offs, sz, version, vername);
            else // add in local and global map 
                if(bind==STB_WEAK) {
                    AddSymbol(weaksymbols, symname, offs, sz, version, vername);
                } else {
                    AddSymbol(mapsymbols, symname, offs, sz, version?version:1, vername);
                }
        }
    }
    checkHookedSymbols(h);
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
int LoadNeededLibs(elfheader_t* h, lib_t *maplib, int local, int bindnow, box86context_t *box86, x86emu_t* emu)
{
    if(h->needed)   // already done
        return 0;
    DumpDynamicRPath(h);
    // update RPATH first
    for (uint32_t i=0; i<h->numDynamic; ++i)
        if(h->Dynamic[i].d_tag==DT_RPATH || h->Dynamic[i].d_tag==DT_RUNPATH) {
            char *rpathref = h->DynStrTab+h->delta+h->Dynamic[i].d_un.d_val;
            char* rpath = rpathref;
            int is_origin = 0;
            while(strstr(rpath, "$ORIGIN")) {
                char* origin = box_strdup(h->path);
                char* p = strrchr(origin, '/');
                if(p) *p = '\0';    // remove file name to have only full path, without last '/'
                char* tmp = (char*)box_calloc(1, strlen(rpath)-strlen("$ORIGIN")+strlen(origin)+1);
                p = strstr(rpath, "$ORIGIN");
                memcpy(tmp, rpath, p-rpath);
                strcat(tmp, origin);
                strcat(tmp, p+strlen("$ORIGIN"));
                if(rpath!=rpathref)
                    box_free(rpath);
                rpath = tmp;
                box_free(origin);
                is_origin = 1;
            }
            while(strstr(rpath, "${ORIGIN}")) {
                char* origin = box_strdup(h->path);
                char* p = strrchr(origin, '/');
                if(p) *p = '\0';    // remove file name to have only full path, without last '/'
                char* tmp = (char*)box_calloc(1, strlen(rpath)-strlen("${ORIGIN}")+strlen(origin)+1);
                p = strstr(rpath, "${ORIGIN}");
                memcpy(tmp, rpath, p-rpath);
                strcat(tmp, origin);
                strcat(tmp, p+strlen("${ORIGIN}"));
                if(rpath!=rpathref)
                    box_free(rpath);
                rpath = tmp;
                box_free(origin);
                is_origin = 1;
            }
            while(strstr(rpath, "${PLATFORM}")) {
                char* platform = box_strdup("i686");
                char* p = strrchr(platform, '/');
                if(p) *p = '\0';    // remove file name to have only full path, without last '/'
                char* tmp = (char*)box_calloc(1, strlen(rpath)-strlen("${PLATFORM}")+strlen(platform)+1);
                p = strstr(rpath, "${PLATFORM}");
                memcpy(tmp, rpath, p-rpath);
                strcat(tmp, platform);
                strcat(tmp, p+strlen("${PLATFORM}"));
                if(rpath!=rpathref)
                    box_free(rpath);
                rpath = tmp;
                box_free(platform);
            }
            if(strchr(rpath, '$')) {
                printf_log(LOG_INFO, "BOX86: Warning, RPATH with $ variable not supported yet (%s)\n", rpath);
            } else {
                printf_log(LOG_DEBUG, "Prepending path \"%s\" to BOX86_LD_LIBRARY_PATH\n", rpath);
                PrependList(&box86->box86_ld_lib, rpath, 1);
            }
            if(rpath!=rpathref)
                box_free(rpath);
        }

    DumpDynamicNeeded(h);
    int cnt = 0;
    // count the number of needed libs, and also grab soname
    for (uint32_t i=0; i<h->numDynamic; ++i) {
        if(h->Dynamic[i].d_tag==DT_NEEDED)
            ++cnt;
        if(h->Dynamic[i].d_tag==DT_SONAME)
            h->soname = h->DynStrTab+h->delta+h->Dynamic[i].d_un.d_val;
    }
    h->needed = new_neededlib(cnt);
    if(h == my_context->elfs[0])
        my_context->neededlibs = h->needed;
    int j=0;
    for (uint32_t i=0; i<h->numDynamic; ++i)
        if(h->Dynamic[i].d_tag==DT_NEEDED)
            h->needed->names[j++] = h->DynStrTab+h->delta+h->Dynamic[i].d_un.d_val;

    // TODO: Add LD_LIBRARY_PATH and RPATH handling
    if(AddNeededLib(maplib, local, bindnow, h->needed, h, box86, emu)) {
        printf_log(LOG_INFO, "Error loading one of needed lib\n");
        if(!allow_missing_libs)
            return 1;   //error...
    }
    return 0;
}

int ElfCheckIfUseTCMallocMinimal(elfheader_t* h)
{
    if(!h)
        return 0;
    for (uint32_t i=0; i<h->numDynamic; ++i)
        if(h->Dynamic[i].d_tag==DT_NEEDED) {
            char *needed = h->DynStrTab+h->delta+h->Dynamic[i].d_un.d_val;
            if(!strcmp(needed, "libtcmalloc_minimal.so.4")) // tcmalloc needs to be the 1st lib loaded
                return 1;
            else if(!strcmp(needed, "libtcmalloc_minimal.so.0")) // tcmalloc needs to be the 1st lib loaded
                return 1;
            else
                return 0;
        }
    return 0;
}

void RefreshElfTLS(elfheader_t* h)
{
    if(h->tlsfilesize) {
        char* dest = (char*)(my_context->tlsdata+my_context->tlssize+h->tlsbase);
        printf_dump(LOG_DEBUG, "Refreshing main TLS block @%p from %p:0x%x\n", dest, (void*)h->tlsaddr, h->tlsfilesize);
        memcpy(dest, (void*)(h->tlsaddr+h->delta), h->tlsfilesize);
        tlsdatasize_t* ptr;
        if ((ptr = (tlsdatasize_t*)pthread_getspecific(my_context->tlskey)) != NULL) {
            // refresh in tlsdata too
            dest = (char*)(ptr->data+h->tlsbase);
            printf_dump(LOG_DEBUG, "Refreshing active TLS block @%p from %p:0x%x\n", dest, (void*)h->tlsaddr, h->tlssize-h->tlsfilesize);
            memcpy(dest, (void*)(h->tlsaddr+h->delta), h->tlsfilesize);
        }
    }
}


void MarkElfInitDone(elfheader_t* h)
{
    if(h)
        h->init_done = 1;
}
void startMallocHook();
void RunElfInitPltResolver(elfheader_t* h, x86emu_t *emu)
{
    if(!h || h->init_done)
        return;
    uintptr_t p = h->initentry + h->delta;
    h->init_done = 1;
    for(int i=0; i<h->needed->size; ++i) {
        library_t *lib = h->needed->libs[i];
        elfheader_t *lib_elf = GetElf(lib);
        if(lib_elf)
            RunElfInitPltResolver(lib_elf, emu);
    }
    printf_log(LOG_DEBUG, "Calling Init for %s %p\n", ElfName(h), (void*)p);
    if(h->initentry)
        RunSafeFunction(p, 3, my_context->argc, my_context->argv, my_context->envv);
    printf_log(LOG_DEBUG, "Done Init for %s\n", ElfName(h));
    // and check init array now
    Elf32_Addr *addr = (Elf32_Addr*)(h->initarray + h->delta);
    for (int i=0; i<h->initarray_sz; ++i) {
        if(addr[i]) {
            printf_log(LOG_DEBUG, "Calling Init[%d] for %s %p\n", i, ElfName(h), (void*)addr[i]);
            RunSafeFunction((uintptr_t)addr[i], 3, my_context->argc, my_context->argv, my_context->envv);
        }
    }

    if(h->malloc_hook_2)
        startMallocHook();

    h->fini_done = 0;   // can be fini'd now (in case it was re-inited)
    printf_log(LOG_DEBUG, "All Init Done for %s\n", ElfName(h));
    return;
}
void RunElfInit(elfheader_t* h, x86emu_t *emu)
{
    if(!h || h->init_done)
        return;
    // reset Segs Cache
    memset(emu->segs_serial, 0, sizeof(emu->segs_serial));
    uintptr_t p = h->initentry + h->delta;
    box86context_t* context = GetEmuContext(emu);
    // Refresh no-file part of TLS in case default value changed
    RefreshElfTLS(h);
    // check if in deferedInit
    if(context->deferredInit) {
        if(context->deferredInitSz==context->deferredInitCap) {
            context->deferredInitCap += 4;
            context->deferredInitList = (elfheader_t**)box_realloc(context->deferredInitList, context->deferredInitCap*sizeof(elfheader_t*));
        }
        context->deferredInitList[context->deferredInitSz++] = h;
        return;
    }
    h->init_done = 1;
    for(int i=0; i<h->needed->size; ++i) {
        library_t *lib = h->needed->libs[i];
        elfheader_t *lib_elf = GetElf(lib);
        if(lib_elf)
            RunElfInit(lib_elf, emu);
    }
    printf_dump(LOG_DEBUG, "Calling Init for %s @%p\n", ElfName(h), (void*)p);
    if(h->initentry)
        RunFunctionWithEmu(emu, 0, p, 3, context->argc, context->argv, context->envv);
    printf_dump(LOG_DEBUG, "Done Init for %s\n", ElfName(h));
    // and check init array now
    Elf32_Addr *addr = (Elf32_Addr*)(h->initarray + h->delta);
    for (size_t i=0; i<h->initarray_sz; ++i) {
        if(addr[i]) {
            printf_dump(LOG_DEBUG, "Calling Init[%zu] for %s @%p\n", i, ElfName(h), (void*)addr[i]);
            RunFunctionWithEmu(emu, 0, (uintptr_t)addr[i], 3, context->argc, context->argv, context->envv);
        }
    }

    if(h->malloc_hook_2)
        startMallocHook();

    h->fini_done = 0;   // can be fini'd now (in case it was re-inited)
    printf_dump(LOG_DEBUG, "All Init Done for %s\n", ElfName(h));
    return;
}

EXPORTDYN
void RunDeferredElfInit(x86emu_t *emu)
{
    box86context_t* context = GetEmuContext(emu);
    if(!context->deferredInit)
        return;
    context->deferredInit = 0;
    if(!context->deferredInitList)
        return;
    int Sz = context->deferredInitSz;
    elfheader_t** List = context->deferredInitList;
    context->deferredInitList = NULL;
    context->deferredInitCap = context->deferredInitSz = 0;
    for (int i=0; i<Sz; ++i)
        RunElfInit(List[i], emu);
    box_free(List);
}

void RunElfFini(elfheader_t* h, x86emu_t *emu)
{
    if(!h || h->fini_done || !h->init_done)
        return;
    h->fini_done = 1;
#ifdef ANDROID
    // TODO: Fix .fini_array on Android
    printf_log(LOG_DEBUG, "Android does not support Fini for %s\n", ElfName(h));
#else
    // first check fini array
    Elf32_Addr *addr = (Elf32_Addr*)(h->finiarray + h->delta);
    for (int i=h->finiarray_sz-1; i>=0; --i) {
        printf_log(LOG_DEBUG, "Calling Fini[%d] for %s %p\n", i, ElfName(h), (void*)addr[i]);
        RunFunctionWithEmu(emu, 0, (uintptr_t)addr[i], 0);
    }
    // then the "old-style" fini
    if(h->finientry) {
        uintptr_t p = h->finientry + h->delta;
        printf_log(LOG_DEBUG, "Calling Fini for %s %p\n", ElfName(h), (void*)p);
        RunFunctionWithEmu(emu, 0, p, 0);
    }
    h->init_done = 0;   // can be re-inited again...
#endif
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

void* GetElfDelta(elfheader_t* h)
{
    return (void*)h->delta;
}

uint32_t GetBaseSize(elfheader_t* h)
{
    return h->memsz;
}

int IsAddressInElfSpace(const elfheader_t* h, uintptr_t addr)
{
    if(!h)
        return 0;
    for(int i=0; i<h->multiblock_n; ++i) {
        uintptr_t base = (uintptr_t)h->multiblocks[i].p;
        uintptr_t end = (uintptr_t)h->multiblocks[i].p + h->multiblocks[i].asize - 1;
        if(addr>=base && addr<=end)
            return 1;
        
    }
    return 0;
}
elfheader_t* FindElfAddress(box86context_t *context, uintptr_t addr)
{
    for (int i=0; i<context->elfsize; ++i)
        if(IsAddressInElfSpace(context->elfs[i], addr))
            return context->elfs[i];
    
    return NULL;
}

const char* FindNearestSymbolName(elfheader_t* h, void* p, uintptr_t* start, uint32_t* sz)
{
    uintptr_t addr = (uintptr_t)p;

    uint32_t distance = 0x7fffffff;
    const char* ret = NULL;
    uintptr_t s = 0;
    uint32_t size = 0;
    if((uintptr_t)p<0x10000)
        return ret;
    #ifdef HAVE_TRACE
    if(!h) {
        if(getProtection((uintptr_t)p)&(PROT_READ)) {
            if(*(uint8_t*)(p)==0xCC && *(uint8_t*)(p+1)=='S' && *(uint8_t*)(p+2)=='C') {
                ret = getBridgeName(*(void**)(p+3+4));
                if(ret) {
                    if(start)
                        *start = (uintptr_t)p;
                    if(sz)
                        *sz = 32;
                }
            }
        }
        return ret;
    }
    #endif
    if(!h || h->fini_done)
        return ret;

    for (uint32_t i=0; (i<h->numSymTab) && (distance!=0); ++i) {
        const char * symname = h->StrTab+h->SymTab[i].st_name;
        uintptr_t offs = h->SymTab[i].st_value + h->delta;

        if(offs<=addr) {
            if(distance>addr-offs) {
                distance = addr-offs;
                ret = symname;
                s = offs;
                size = h->SymTab[i].st_size;
            }
        }
    }
    for (uint32_t i=0; (i<h->numDynSym) && (distance!=0); ++i) {
        const char * symname = h->DynStr+h->DynSym[i].st_name;
        uintptr_t offs = h->DynSym[i].st_value + h->delta;

        if(offs<=addr) {
            if(distance>addr-offs) {
                distance = addr-offs;
                ret = symname;
                s = offs;
                size = h->DynSym[i].st_size;
            }
        }
    }

    if(start)
        *start = s;
    if(sz)
        *sz = size;

    return ret;
}

const char* VersionedName(const char* name, int ver, const char* vername)
{
    if(ver<0)
        return name;
    const char *v=NULL;
    if(ver==0)
        v="";
    if(ver==1)
        v="*";
    if(!v && !vername)
        return name;
    if(ver>1)
        v = vername;
    char buf[strlen(name)+strlen(v)+1+1];
    strcpy(buf, name);
    strcat(buf, "@");
    strcat(buf, v);
    return AddDictionnary(my_context->versym, buf);
}

int SameVersionedSymbol(const char* name1, int ver1, const char* vername1, const char* name2, int ver2, const char* vername2)
{
    if(strcmp(name1, name2))    //name are different, no need to go further
        return 0;
    if((ver1<0) || (ver2<0))    // don't check version, so ok
        return 1;
    if(ver1==ver2 && ver1<2)    // same ver (local or global), ok
        return 1;
    if(ver1==0 || ver2==0)  // one is local, the other is not, no match
        return 0;
    if(ver1==1 || ver2==1)  // one if global, ok
        return 1;
    if(!strcmp(vername1, vername2))  // same vername
        return 1;
    return 0;
}

void* GetDTatOffset(box86context_t* context, int index, int offset)
{
    return (void*)((char*)GetTLSPointer(context, context->elfs[index])+offset);
}

int32_t GetTLSBase(elfheader_t* h)
{
    return h?h->tlsbase:0;
}

uint32_t GetTLSSize(elfheader_t* h)
{
    return h?h->tlssize:0;
}

void* GetTLSPointer(box86context_t* context, elfheader_t* h)
{
    if(!h->tlssize)
        return NULL;
    tlsdatasize_t* ptr;
    if ((ptr = (tlsdatasize_t*)pthread_getspecific(context->tlskey)) == NULL) {
        ptr = (tlsdatasize_t*)fillTLSData(context);
    }
    if(ptr->tlssize != context->tlssize)
        ptr = (tlsdatasize_t*)resizeTLSData(context, ptr);
    return ptr->data+h->tlsbase;
}

void* GetDynamicSection(elfheader_t* h)
{
    if(!h)
        return NULL;
    return h->Dynamic;
}

void* ElfGetBrk(elfheader_t* h)
{
    if(!h)
        return NULL;
    return (void*)(h->bss+h->delta+h->bsssz);
}

void* ElfSetBrk(void* newbrk)
{
    if(!my_context)
        return NULL;
    if(!my_context->elfsize)
        return NULL;
    elfheader_t* h = my_context->elfs[0];
    void* ret = my_context->brk + my_context->brksz;
    if(!newbrk)
        return ret;
    // compute the added size
    intptr_t added = (intptr_t)newbrk - (intptr_t)my_context->brk;
    if(added<=0) // cannot remove bss
        return ret;
    //check if elf is 1 big memory and splitted
    if(h->multiblock_n!=1) {
        // meh
        printf_log(LOG_NONE, "Error, using brk on multi-part memory elf is not allowed for now");
    } else {
        printf_log(LOG_DEBUG, "brk change, start=%p with reserve=0x%x, was 0x%x long, will be 0x%x now => %p\n", my_context->brk, h->reserve, my_context->brksz, added, my_context->brk+added);
        if(added>0 && (uint32_t)added<h->reserve) {
            my_context->brksz = added;
            return newbrk;
        }
        void* newmem = mremap(h->memory, h->memsz+h->reserve+my_context->brksz, h->memsz+h->reserve+added, 0);
        if(newmem!=(void*)-1) {
            if(added>my_context->brksz)
                setProtection((uintptr_t)my_context->brk + my_context->brksz, added-my_context->brksz, PROT_READ|PROT_WRITE);
            else
                freeProtection((uintptr_t)my_context->brk + added, my_context->brksz - added);
            my_context->brksz = added;
            ret = my_context->brk + added;
        } else {
            printf_log(LOG_DEBUG, "failed to remap\n");
        }
    }
    return ret;
}

#ifdef DYNAREC
dynablock_t* GetDynablocksFromAddress(box86context_t *context, uintptr_t addr)
{
    (void)context;
    // if we are here, the there is not block in standard "space"
    /*dynablocklist_t* ret = getDBFromAddress(addr);
    if(ret) {
        return ret;
    }*/
    if(box86_dynarec_forced) {
        addDBFromAddressRange(addr, 1);
        return getDB(addr);
    }
    //check if address is in an elf... if yes, grant a block (should I warn)
    Dl_info info;
    if(dladdr((void*)addr, &info)) {
        dynarec_log(LOG_INFO, "Address %p is in a native Elf memory space (function \"%s\" in %s)\n", (void*)addr, info.dli_sname, info.dli_fname);
        return NULL;
    }
    dynarec_log(LOG_INFO, "Address %p not found in Elf memory and is not a native call wrapper\n", (void*)addr);
    return NULL;
}
#endif

typedef struct my_dl_phdr_info_s {
    void*           dlpi_addr;
    const char*     dlpi_name;
    Elf32_Phdr*     dlpi_phdr;
    int             dlpi_phnum;
} my_dl_phdr_info_t;

static int dl_iterate_phdr_callback(x86emu_t *emu, void* F, my_dl_phdr_info_t *info, size_t size, void* data)
{
    int ret = RunFunctionWithEmu(emu, 0, (uintptr_t)F, 3, info, size, data);
    return ret;
}

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// dl_iterate_phdr ...
#define GO(A)   \
static uintptr_t my_dl_iterate_phdr_fct_##A = 0;                                    \
static int my_dl_iterate_phdr_##A(struct dl_phdr_info* a, size_t b, void* c)        \
{                                                                                   \
    if(!a->dlpi_name)                                                               \
        return 0;                                                                   \
    if(!a->dlpi_name[0]) /*don't send informations about box86 itself*/             \
        return 0;                                                                   \
    return RunFunctionFmt(my_dl_iterate_phdr_fct_##A, "pLp", a, b, c);  \
}
SUPER()
#undef GO
static void* find_dl_iterate_phdr_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_dl_iterate_phdr_fct_##A == (uintptr_t)fct) return my_dl_iterate_phdr_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_dl_iterate_phdr_fct_##A == 0) {my_dl_iterate_phdr_fct_##A = (uintptr_t)fct; return my_dl_iterate_phdr_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for elfloader dl_iterate_phdr callback\n");
    return NULL;
}
#undef SUPER

EXPORT int my_dl_iterate_phdr(x86emu_t *emu, void* F, void *data) {
    printf_log(LOG_INFO, "Warning: call to partially implemented dl_iterate_phdr(%p, %p)\n", F, data);
    box86context_t *context = GetEmuContext(emu);
    const char* empty = "";
    int ret = 0;
    for (int idx=0; idx<context->elfsize; ++idx) {
        my_dl_phdr_info_t info;
        info.dlpi_addr = GetElfDelta(context->elfs[idx]);
        info.dlpi_name = idx?context->elfs[idx]->name:empty;    //1st elf is program, and this one doesn't get a name
        info.dlpi_phdr = context->elfs[idx]->PHEntries;
        info.dlpi_phnum = context->elfs[idx]->numPHEntries;
        if((ret = dl_iterate_phdr_callback(emu, F, &info, sizeof(info), data))) {
            return ret;
        }
    }
    // and now, go on native version
    ret = dl_iterate_phdr(find_dl_iterate_phdr_Fct(F), data);
    return ret;
}

void ResetSpecialCaseMainElf(elfheader_t* h)
{
    Elf32_Sym *sym = NULL;
     for (uint32_t i=0; i<h->numDynSym; ++i) {
        if(h->DynSym[i].st_info == 17) {
            sym = h->DynSym+i;
            const char * symname = h->DynStr+sym->st_name;
            if(strcmp(symname, "_IO_2_1_stderr_")==0 && ((void*)sym->st_value+h->delta)) {
                memcpy((void*)sym->st_value+h->delta, stderr, sym->st_size);
                my__IO_2_1_stderr_ = (void*)sym->st_value+h->delta;
                printf_log(LOG_DEBUG, "BOX86: Set @_IO_2_1_stderr_ to %p\n", my__IO_2_1_stderr_);
            } else
            if(strcmp(symname, "_IO_2_1_stdin_")==0 && ((void*)sym->st_value+h->delta)) {
                memcpy((void*)sym->st_value+h->delta, stdin, sym->st_size);
                my__IO_2_1_stdin_ = (void*)sym->st_value+h->delta;
                printf_log(LOG_DEBUG, "BOX86: Set @_IO_2_1_stdin_ to %p\n", my__IO_2_1_stdin_);
            } else
            if(strcmp(symname, "_IO_2_1_stdout_")==0 && ((void*)sym->st_value+h->delta)) {
                memcpy((void*)sym->st_value+h->delta, stdout, sym->st_size);
                my__IO_2_1_stdout_ = (void*)sym->st_value+h->delta;
                printf_log(LOG_DEBUG, "BOX86: Set @_IO_2_1_stdout_ to %p\n", my__IO_2_1_stdout_);
            } else
            if(strcmp(symname, "_IO_stderr_")==0 && ((void*)sym->st_value+h->delta)) {
                memcpy((void*)sym->st_value+h->delta, stderr, sym->st_size);
                my__IO_2_1_stderr_ = (void*)sym->st_value+h->delta;
                printf_log(LOG_DEBUG, "BOX86: Set @_IO_stderr_ to %p\n", my__IO_2_1_stderr_);
            } else
            if(strcmp(symname, "_IO_stdin_")==0 && ((void*)sym->st_value+h->delta)) {
                memcpy((void*)sym->st_value+h->delta, stdin, sym->st_size);
                my__IO_2_1_stdin_ = (void*)sym->st_value+h->delta;
                printf_log(LOG_DEBUG, "BOX86: Set @_IO_stdin_ to %p\n", my__IO_2_1_stdin_);
            } else
            if(strcmp(symname, "_IO_stdout_")==0 && ((void*)sym->st_value+h->delta)) {
                memcpy((void*)sym->st_value+h->delta, stdout, sym->st_size);
                my__IO_2_1_stdout_ = (void*)sym->st_value+h->delta;
                printf_log(LOG_DEBUG, "BOX86: Set @_IO_stdout_ to %p\n", my__IO_2_1_stdout_);
            }
        }
    }
}


void CreateMemorymapFile(box86context_t* context, int fd)
{
    char buff[1024];
    struct stat st;
    int dummy;
    (void)dummy;

    elfheader_t *h = context->elfs[0];
    if (stat(h->path, &st)) {
        printf_log(LOG_INFO, "Failed to stat file %s (creating memory maps \"file\")!", h->path);
        // Some constants, to have "valid" values
        st.st_dev = makedev(0x03, 0x00);
        st.st_ino = 0;
    }
    // TODO: create heap entry?

    for (int i=0; i<h->numPHEntries; ++i) {
        if (h->PHEntries[i].p_memsz == 0) continue;

        sprintf(buff, "%08x-%08x %c%c%c%c %08x %02x:%02x %ld %s\n", (uintptr_t)h->PHEntries[i].p_vaddr + h->delta,
            (uintptr_t)h->PHEntries[i].p_vaddr + h->PHEntries[i].p_memsz + h->delta,
            (h->PHEntries[i].p_type & (PF_R|PF_X) ? 'r':'-'), (h->PHEntries[i].p_type & PF_W ? 'w':'-'),
            (h->PHEntries[i].p_type & PF_X ? 'x':'-'), 'p', // p for private or s for shared
            (uintptr_t)h->PHEntries[i].p_offset,
            major(st.st_dev), minor(st.st_dev), st.st_ino, h->path);
        
        dummy = write(fd, buff, strlen(buff));
    }
    // create stack entry
    sprintf(buff, "%08x-%08x %c%c%c%c %08x %02x:%02x %ld %s\n", 
        (uintptr_t)context->stack, (uintptr_t)context->stack+context->stacksz,
        'r','w','-','p', 0, 0, 0, 0L, "[stack]");
    dummy = write(fd, buff, strlen(buff));
}

void ElfAttachLib(elfheader_t* head, library_t* lib)
{
    if(!head)
        return;
    head->lib = lib;
}

kh_mapsymbols_t* GetMapSymbols(elfheader_t* h)
{
    if(!h)
        return NULL;
    return h->mapsymbols;
}
kh_mapsymbols_t* GetWeakSymbols(elfheader_t* h)
{
    if(!h)
        return NULL;
    return h->weaksymbols;
}
kh_mapsymbols_t* GetLocalSymbols(elfheader_t* h)
{
    if(!h)
        return NULL;
    return h->localsymbols;
}

typedef struct search_symbol_s{
    const char* name;
    void*       addr;
    void*       lib;
} search_symbol_t;
int dl_iterate_phdr_findsymbol(struct dl_phdr_info* info, size_t size, void* data)
{
    (void)size;
    search_symbol_t* s = (search_symbol_t*)data;

    for(int j = 0; j<info->dlpi_phnum; ++j) {
        if (info->dlpi_phdr[j].p_type == PT_DYNAMIC) {
            //ElfW(Sym)* sym = NULL;
            //ElfW(Word) sym_cnt = 0;
            ElfW(Verdef)* verdef = NULL;
            ElfW(Word) verdef_cnt = 0;
            char *strtab = NULL;
            ElfW(Dyn)* dyn = (ElfW(Dyn)*)(info->dlpi_addr +  info->dlpi_phdr[j].p_vaddr); //Dynamic Section
            // grab the needed info
            while(dyn->d_tag != DT_NULL) {
                switch(dyn->d_tag) {
                    case DT_STRTAB:
                        strtab = (char *)(dyn->d_un.d_ptr);
                        break;
                    case DT_VERDEF:
                        verdef = (ElfW(Verdef)*)(info->dlpi_addr +  dyn->d_un.d_ptr);
                        break;
                    case DT_VERDEFNUM:
                        verdef_cnt = dyn->d_un.d_val;
                        break;
                }
                ++dyn;
            }
            if(strtab && verdef && verdef_cnt) {
                if((uintptr_t)strtab < (uintptr_t)info->dlpi_addr) // this test is need for linux-vdso on PI and some other OS (looks like a bug to me)
                    strtab=(char*)((uintptr_t)strtab + info->dlpi_addr);
                // Look fr all defined versions now
                ElfW(Verdef)* v = verdef;
                while(v) {
                    ElfW(Verdaux)* vda = (ElfW(Verdaux)*)(((uintptr_t)v) + v->vd_aux);
                    if(v->vd_version>0 && !v->vd_flags)
                        for(int i=0; i<v->vd_cnt; ++i) {
                            const char* vername = (strtab+vda->vda_name);
                            if(vername && vername[0] && (s->addr = dlvsym(s->lib, s->name, vername))) {
                                printf_log(/*LOG_DEBUG*/LOG_INFO, "Found symbol with version %s, value = %p\n", vername, s->addr);
                                return 1;   // stop searching
                            }
                            vda = (ElfW(Verdaux)*)(((uintptr_t)vda) + vda->vda_next);
                        }
                    v = v->vd_next?(ElfW(Verdef)*)((uintptr_t)v + v->vd_next):NULL;
                }
            }
        }
    }
    return 0;
}

void* GetNativeSymbolUnversioned(void* lib, const char* name)
{
    // try to find "name" in loaded elf, whithout checking for the symbol version (like dlsym, but no version check)
    search_symbol_t s;
    s.name = name;
    s.addr = NULL;
    if(lib)
        s.lib = lib;
    else
        s.lib = my_context->box86lib;
    printf_log(LOG_INFO, "Look for %s in loaded elfs\n", name);
    dl_iterate_phdr(dl_iterate_phdr_findsymbol, &s);
    return s.addr;
}

kh_defaultversion_t* GetGlobalDefaultVersion(elfheader_t* h)
{
    return h?h->globaldefver:NULL;
}
kh_defaultversion_t* GetWeakDefaultVersion(elfheader_t* h)
{
    return h?h->weakdefver:NULL;
}


uintptr_t pltResolver = ~(uintptr_t)0;
EXPORT void PltResolver(x86emu_t* emu)
{
    uintptr_t addr = Pop32(emu);
    int slot = (int)Pop32(emu);
    elfheader_t *h = (elfheader_t*)addr;
    printf_dump(LOG_DEBUG, "PltResolver: Addr=%p, Slot=%d Return=%p: elf is %s (VerSym=%p)\n", (void*)addr, slot, *(void**)(R_ESP), h->name, h->VerSym);

    Elf32_Rel * rel = (Elf32_Rel *)(h->jmprel + h->delta + slot);

    Elf32_Sym *sym = &h->DynSym[ELF32_R_SYM(rel->r_info)];
    int bind = ELF32_ST_BIND(sym->st_info);
    const char* symname = SymName(h, sym);
    int version = h->VerSym?((Elf32_Half*)((uintptr_t)h->VerSym+h->delta))[ELF32_R_SYM(rel->r_info)]:-1;
    if(version!=-1) version &= 0x7fff;
    const char* vername = GetSymbolVersion(h, version);
    uint32_t *p = (uint32_t*)(rel->r_offset + h->delta);
    uintptr_t offs = 0;
    uintptr_t end = 0;

    library_t* lib = h->lib;
    lib_t* local_maplib = GetMaplib(lib);
    const char* globdefver = (bind==STB_WEAK)?NULL:GetMaplibDefaultVersion(my_context->maplib, (my_context->maplib==local_maplib)?NULL:local_maplib, 0, symname);
    const char* weakdefver = (bind==STB_WEAK)?GetMaplibDefaultVersion(my_context->maplib, (my_context->maplib==local_maplib)?NULL:local_maplib, 1, symname):NULL;
    GetGlobalSymbolStartEnd(my_context->maplib, symname, &offs, &end, h, version, vername, globdefver, weakdefver);
    if(!offs && !end && local_maplib) {
        GetGlobalSymbolStartEnd(local_maplib, symname, &offs, &end, h, version, vername, globdefver, weakdefver);
    }
    if(!offs && !end && !version)
        GetGlobalSymbolStartEnd(my_context->maplib, symname, &offs, &end, h, -1, NULL, globdefver, weakdefver);


    if (!offs) {
        printf_log(LOG_NONE, "Error: PltResolver: Symbol %s(ver %d: %s%s%s) not found, cannot apply R_386_JMP_SLOT %p (%p) in %s\n", symname, version, symname, vername?"@":"", vername?vername:"", p, *(void**)p, h->name);
        emu->quit = 1;
        return;
    } else {
        elfheader_t* sym_elf = FindElfAddress(my_context, offs);
        if(sym_elf && sym_elf!=my_context->elfs[0] && !sym_elf->init_done) {
            printf_dump(LOG_DEBUG, "symbol %s from %s but elf not initialized yet, run Init now (from %s)\n", symname, ElfName(sym_elf), ElfName(h));
            RunElfInitPltResolver(sym_elf, emu);
        }
        offs = (uintptr_t)getAlternate((void*)offs);

        if(p) {
            printf_dump(LOG_DEBUG, "            Apply %s R_386_JMP_SLOT %p with sym=%s(ver %d: %s%s%s) (%p -> %p / %s)\n", (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), p, symname, version, symname, vername?"@":"", vername?vername:"",*(void**)p, (void*)offs, ElfName(FindElfAddress(my_context, offs)));
            *p = offs;
        } else {
            printf_log(LOG_NONE, "PltResolver: Warning, Symbol %s(ver %d: %s%s%s) found, but Jump Slot Offset is NULL \n", symname, version, symname, vername?"@":"", vername?vername:"");
        }
    }

    // jmp to function
    R_EIP = offs;
}

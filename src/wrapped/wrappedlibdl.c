#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <elf.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "emu/x86emu_private.h"
#include "library.h"
#include "librarian.h"
#include "box86context.h"
#include "elfloader.h"
#include "elfs/elfloader_private.h"

typedef struct dlprivate_s {
    library_t   **libs;
    int         *count;
    int         *dlopened;
    int         lib_sz;
    int         lib_cap;
    char*       last_error;
} dlprivate_t;

dlprivate_t *NewDLPrivate() {
    dlprivate_t* dl =  (dlprivate_t*)box_calloc(1, sizeof(dlprivate_t));
    return dl;
}
void FreeDLPrivate(dlprivate_t **lib) {
    box_free((*lib)->last_error);
    box_free(*lib);
}

void* my_dlopen(x86emu_t* emu, void *filename, int flag) EXPORT;
void* my_dlmopen(x86emu_t* emu, void* mlid, void *filename, int flag) EXPORT;
char* my_dlerror(x86emu_t* emu) EXPORT;
void* my_dlsym(x86emu_t* emu, void *handle, void *symbol) EXPORT;
int my_dlclose(x86emu_t* emu, void *handle) EXPORT;
int my_dladdr(x86emu_t* emu, void *addr, void *info) EXPORT;
int my_dladdr1(x86emu_t* emu, void *addr, void *info, void** extra_info, int flags) EXPORT;
void* my_dlvsym(x86emu_t* emu, void *handle, void *symbol, const char* vername) EXPORT;
int my_dlinfo(x86emu_t* emu, void* handle, int request, void* info) EXPORT;

#define LIBNAME libdl
const char* libdlName =
#ifdef ANDROID
    "libdl.so"
#else
    "libdl.so.2"
#endif
    ;

// define all standard library functions
#include "wrappedlib_init.h"

#define CLEARERR    if(dl->last_error) box_free(dl->last_error); dl->last_error = NULL;

extern int box86_zoom;
// Implementation
void* my_dlopen(x86emu_t* emu, void *filename, int flag)
{
    //void *dlopen(const char *filename, int flag);
    // TODO, handling special values for filename, like RTLD_SELF?
    // TODO, handling flags?
    library_t *lib = NULL;
    dlprivate_t *dl = my_context->dlprivate;
    int dlopened = 0;
    int is_local = (flag&0x100)?0:1;  // if not global, then local, and that means symbols are not put in the global "pot" for other libs
    CLEARERR
    if(filename) {
        char* rfilename = (char*)alloca(MAX_PATH);
        strcpy(rfilename, (char*)filename);
        if(box86_zoom && strstr(rfilename, "/libturbojpeg.so")) {
            void* sys = my_dlopen(emu, "libturbojpeg.so.0", flag);
            if(sys)
                return sys;
        }
        if(!strcmp(rfilename, "/usr/lib/i386-linux-gnu/d3d")) {
            void* sys = my_dlopen(emu, "d3dadapter9.so.1", flag);
            if(sys)
                return sys;
        }
        printf_dlsym(LOG_DEBUG, "BOX86: Call to dlopen(\"%s\"/%p, %X)\n", rfilename, filename, flag);
        // Transform any ${...} that maight be present
        while(strstr(rfilename, "${ORIGIN}")) {
            char* origin = box_strdup(my_context->fullpath);
            char* p = strrchr(origin, '/');
            if(p) *p = '\0';    // remove file name to have only full path, without last '/'
            char* tmp = (char*)box_calloc(1, strlen(rfilename)-strlen("${ORIGIN}")+strlen(origin)+1);
            p = strstr(rfilename, "${ORIGIN}");
            memcpy(tmp, rfilename, p-rfilename);
            strcat(tmp, origin);
            strcat(tmp, p+strlen("${ORIGIN}"));
            strcpy(rfilename, tmp);
            box_free(tmp);
            box_free(origin);
        }
        // check if alread dlopenned...
        for (int i=0; i<dl->lib_sz; ++i) {
            if(IsSameLib(dl->libs[i], rfilename)) {
                if(dl->count[i]==0 && dl->dlopened[i]) {   // need to lauch init again!
                    if(flag&0x4) {
                        printf_dlsym(LOG_DEBUG, " => not present anymore\n");
                        return NULL;    // don't re-open in RTLD_NOLOAD mode
                    }
                    int idx = GetElfIndex(dl->libs[i]);
                    if(idx!=-1) {
                            printf_dlsym(LOG_DEBUG, "dlopen: Recycling, calling Init for %p (%s)\n", (void*)(i+1), rfilename);
                        ReloadLibrary(dl->libs[i], emu);    // reset memory image, redo reloc, run inits
                    }
                }
                if(!(flag&0x4))
                    dl->count[i] = dl->count[i]+1;
                printf_dlsym(LOG_DEBUG, "dlopen: Recycling %s/%p count=%d (dlopened=%d, elf_index=%d)\n", rfilename, (void*)(i+1), dl->count[i], dl->dlopened[i], GetElfIndex(dl->libs[i]));
                return (void*)(i+1);
            }
        }
        if(flag&0x4) {   //RTLD_NOLOAD is just a "check" if lib is already loaded
            printf_dlsym(LOG_DEBUG, " => not present\n");
            return NULL;
        }
        dlopened = (GetLibInternal(rfilename)==NULL);
        // Then open the lib
        const char* libs[] = {rfilename};
        my_context->deferedInit = 1;
        int bindnow = (flag&0x2 && !allow_missing_symbols)?1:0;
        int old_missing = allow_missing_libs;
        if(flag&0x01)   // RTLD_LAZY
            allow_missing_libs = 1;
        if(AddNeededLib(NULL, NULL, NULL, is_local, bindnow, libs, 1, my_context, emu)) {
            allow_missing_libs = old_missing;
            printf_dlsym(strchr(rfilename,'/')?LOG_DEBUG:LOG_INFO, "Warning: Cannot dlopen(\"%s\"/%p, %X)\n", rfilename, filename, flag);
            if(!dl->last_error)
                dl->last_error = box_malloc(129);
            snprintf(dl->last_error, 129, "Cannot dlopen(\"%s\"/%p, %X)\n", rfilename, filename, flag);
            return NULL;
        }
        allow_missing_libs = old_missing;
        lib = GetLibInternal(rfilename);
        RunDeferedElfInit(emu);
    } else {
        // check if already dlopenned...
        for (int i=0; i<dl->lib_sz; ++i) {
            if(!dl->libs[i]) {
                dl->count[i] = dl->count[i]+1;
                return (void*)(i+1);
            }
        }
        printf_dlsym(LOG_DEBUG, "Call to dlopen(NULL, %X)\n", flag);
    }
    //get the lib and add it to the collection
    
    if(dl->lib_sz == dl->lib_cap) {
        dl->lib_cap += 4;
        dl->libs = (library_t**)box_realloc(dl->libs, sizeof(library_t*)*dl->lib_cap);
        dl->count = (int*)box_realloc(dl->count, sizeof(int)*dl->lib_cap);
        dl->dlopened = (int*)box_realloc(dl->dlopened, sizeof(int)*dl->lib_cap);
        // memset count...
        memset(dl->count+dl->lib_sz, 0, (dl->lib_cap-dl->lib_sz)*sizeof(int));
    }
    dl->libs[dl->lib_sz] = lib;
    dl->count[dl->lib_sz] = dl->count[dl->lib_sz]+1;
    dl->dlopened[dl->lib_sz] = dlopened;
    printf_dlsym(LOG_DEBUG, "dlopen: New handle %p (%s), dlopened=%d\n", (void*)(dl->lib_sz+1), (char*)filename, dlopened);
    return (void*)(++dl->lib_sz);
}
void* my_dlmopen(x86emu_t* emu, void* lmid, void *filename, int flag)
{
    if(lmid) {
        printf_dlsym(LOG_INFO, "Warning, dlmopen(%p, %p(\"%s\"), 0x%x) called with lmid not LMID_ID_BASE (unsupported)\n", lmid, filename, filename?(char*)filename:"self", flag);
    }
    // lmid is ignored for now...
    return my_dlopen(emu, filename, flag);
}
char* my_dlerror(x86emu_t* emu)
{
    dlprivate_t *dl = my_context->dlprivate;
    return dl->last_error;
}

KHASH_SET_INIT_INT(libs);

int recursive_dlsym_lib(kh_libs_t* collection, library_t* lib, const char* rsymbol, uintptr_t *start, uintptr_t *end, int version, const char* vername)
{
    if(!lib)
        return 0;
    khint_t k = kh_get(libs, collection, (uintptr_t)lib);
    if(k != kh_end(collection))
        return 0;
    int ret;
    kh_put(libs, collection, (uintptr_t)lib, &ret);
    // TODO: should use librarian functions instead!
    int weak;
    // look in the library itself
    if(lib->getglobal(lib, rsymbol, start, end, 0, &weak, version, vername, 1))
        return 1;
    if(lib->getweak(lib, rsymbol, start, end, 0, &weak, version, vername, 1))
        return 1;
    // look in other libs
    int n = GetNeededLibN(lib);
    for (int i=0; i<n; ++i) {
        library_t *l = GetNeededLib(lib, i);
        if(recursive_dlsym_lib(collection, l, rsymbol, start, end, version, vername))
            return 1;
    }
        
    return 0;
}

int my_dlsym_lib(library_t* lib, const char* rsymbol, uintptr_t *start, uintptr_t *end, int version, const char* vername)
{
    kh_libs_t *collection = kh_init(libs);
    int ret = recursive_dlsym_lib(collection, lib, rsymbol, start, end, version, vername);
    kh_destroy(libs, collection);

    return ret;
}
void* my_dlsym(x86emu_t* emu, void *handle, void *symbol)
{
    dlprivate_t *dl = my_context->dlprivate;
    uintptr_t start = 0, end = 0;
    char* rsymbol = (char*)symbol;
    CLEARERR
    printf_dlsym(LOG_DEBUG, "Call to dlsym(%p, \"%s\")%s", handle, rsymbol, dlsym_error?"":"\n");
    if(handle==NULL) {
        // special case, look globably
        if(GetGlobalSymbolStartEnd(my_context->maplib, rsymbol, &start, &end, NULL, -1, NULL)) {
            printf_dlsym(LOG_NEVER, "%p\n", (void*)start);
            return (void*)start;
        }
        if(!dl->last_error)
            dl->last_error = box_malloc(129);
        snprintf(dl->last_error, 129, "Symbol \"%s\" not found in %p)\n", rsymbol, handle);
        printf_dlsym(LOG_NEVER, "%p\n", NULL);
        return NULL;
    }
    if(handle==(void*)0xFFFFFFFF) {
        // special case, look globably but no self (RTLD_NEXT)
        elfheader_t *elf = FindElfAddress(my_context, *(uint32_t*)R_ESP); // use return address to guess "self"
        if(GetNoSelfSymbolStartEnd(my_context->maplib, rsymbol, &start, &end, elf, 0, -1, NULL)) {
            printf_dlsym(LOG_NEVER, "%p\n", (void*)start);
            return (void*)start;
        }
        if(!dl->last_error)
            dl->last_error = box_malloc(129);
        snprintf(dl->last_error, 129, "Symbol \"%s\" not found in %p)\n", rsymbol, handle);
        printf_dlsym(LOG_NEVER, "%p\n", NULL);
        return NULL;
    }
    int nlib = (int)handle;
    --nlib;
    if(nlib<0 || nlib>=dl->lib_sz) {
        if(!dl->last_error)
            dl->last_error = box_malloc(129);
        snprintf(dl->last_error, 129, "Bad handle %p)\n", handle);
        printf_dlsym(LOG_NEVER, "%p\n", NULL);
        return NULL;
    }
    if(dl->count[nlib]==0) {
        if(!dl->last_error)
            dl->last_error = box_malloc(129);
        snprintf(dl->last_error, 129, "Bad handle %p (already closed))\n", handle);
        printf_dlsym(LOG_NEVER, "%p\n", (void*)NULL);
        return NULL;
    }
    if(dl->libs[nlib]) {
        if(my_dlsym_lib(dl->libs[nlib], rsymbol, &start, &end, -1, NULL)==0) {
            // not found
            printf_dlsym(LOG_NEVER, "%p\nCall to dlsym(%s, \"%s\") Symbol not found\n", NULL, GetNameLib(dl->libs[nlib]), rsymbol);
            printf_log(LOG_DEBUG, " Symbol not found\n");
            if(!dl->last_error)
                dl->last_error = box_malloc(129);
            snprintf(dl->last_error, 129, "Symbol \"%s\" not found in %p(%s)", rsymbol, handle, GetNameLib(dl->libs[nlib]));
            return NULL;
        }
    } else {
        const char* defver = GetDefaultVersion(my_context->globaldefver, rsymbol);
        if(!defver) defver = GetDefaultVersion(my_context->weakdefver, rsymbol);
        if(GetGlobalSymbolStartEnd(my_context->maplib, rsymbol, &start, &end, NULL, -1, defver)) {
            printf_dlsym(LOG_NEVER, "%p\n", (void*)start);
            return (void*)start;
        }
        // not found
        printf_dlsym(LOG_NEVER, "%p\nCall to dlsym(%s, \"%s\") Symbol not found\n", NULL, "Self", rsymbol);
        printf_log(LOG_DEBUG, " Symbol not found\n");
        if(!dl->last_error)
            dl->last_error = box_malloc(129);
        snprintf(dl->last_error, 129, "Symbol \"%s\" not found in %p)\n", rsymbol, handle);
        return NULL;
    }
    printf_dlsym(LOG_NEVER, "%p\n", (void*)start);
    return (void*)start;
}
int my_dlclose(x86emu_t* emu, void *handle)
{
        printf_dlsym(LOG_DEBUG, "Call to dlclose(%p)\n", handle);
    dlprivate_t *dl = my_context->dlprivate;
    CLEARERR
    int nlib = (int)handle;
    --nlib;
    if(nlib<0 || nlib>=dl->lib_sz) {
        if(!dl->last_error)
            dl->last_error = box_malloc(129);
        snprintf(dl->last_error, 129, "Bad handle %p)\n", handle);
        printf_dlsym(LOG_DEBUG, "dlclose: %s\n", dl->last_error);
        return -1;
    }
    if(dl->count[nlib]==0) {
        if(!dl->last_error)
            dl->last_error = box_malloc(129);
        snprintf(dl->last_error, 129, "Bad handle %p (already closed))\n", handle);
        printf_dlsym(LOG_DEBUG, "dlclose: %s\n", dl->last_error);
        return -1;
    }
    dl->count[nlib] = dl->count[nlib]-1;
    if(dl->count[nlib]==0 && dl->dlopened[nlib]) {   // need to call Fini...
        int idx = GetElfIndex(dl->libs[nlib]);
        if(idx!=-1) {
            printf_dlsym(LOG_DEBUG, "dlclose: Call to Fini for %p\n", handle);
            RunElfFini(my_context->elfs[idx], emu);
            InactiveLibrary(dl->libs[nlib]);
        }
    }
    return 0;
}
int my_dladdr1(x86emu_t* emu, void *addr, void *i, void** extra_info, int flags)
{
    //int dladdr(void *addr, Dl_info *info);
    dlprivate_t *dl = my_context->dlprivate;
    CLEARERR
    Dl_info *info = (Dl_info*)i;
    printf_log(LOG_DEBUG, "Warning: partially unimplement call to dladdr/dladdr1(%p, %p, %p, %d)\n", addr, info, extra_info, flags);
    
    //emu->quit = 1;
    #ifndef ANDROID
    void* native = GetNativeFnc((uintptr_t)addr);
    if(native)
        return dladdr1(native, info, extra_info, flags);
    #endif
    library_t* lib = NULL;
    info->dli_saddr = NULL;
    info->dli_fname = NULL;
    info->dli_sname = FindSymbolName(my_context->maplib, addr, &info->dli_saddr, NULL, &info->dli_fname, &info->dli_fbase, &lib);
    #ifndef RTLD_DL_SYMENT
    #define RTLD_DL_SYMENT 1
    #endif
    #ifndef RTLD_DL_LINKMAP
    #define RTLD_DL_LINKMAP 2
    #endif
    printf_log(LOG_DEBUG, "     dladdr return saddr=%p, fname=\"%s\", sname=\"%s\"\n", info->dli_saddr, info->dli_sname?info->dli_sname:"", info->dli_fname?info->dli_fname:"");
    if(flags==RTLD_DL_SYMENT) {
        printf_log(LOG_INFO, "Warning, unimplement call to dladdr1 with RTLD_DL_SYMENT flags\n");
    } else if (flags==RTLD_DL_LINKMAP) {
        printf_log(LOG_INFO, "Warning, partially unimplemented call to dladdr1 with RTLD_DL_LINKMAP flags\n");
        *(linkmap_t**)extra_info = getLinkMapLib(lib);
    }
    return (info->dli_sname)?1:0;   // success is non-null here...
}
int my_dladdr(x86emu_t* emu, void *addr, void *i)
{
    return my_dladdr1(emu, addr, i, NULL, 0);
}
void* my_dlvsym(x86emu_t* emu, void *handle, void *symbol, const char* vername)
{
    dlprivate_t *dl = my_context->dlprivate;
    int version = (vername)?2:-1;
    uintptr_t start, end;
    char* rsymbol = (char*)symbol;
    CLEARERR
    printf_dlsym(LOG_DEBUG, "Call to dlvsym(%p, \"%s\", %s)%s", handle, rsymbol, vername?vername:"(nil)", dlsym_error?" => ":"\n");
    if(handle==NULL) {
        // special case, look globably
        if(GetGlobalSymbolStartEnd(my_context->maplib, rsymbol, &start, &end, NULL, version, vername)) {
            printf_dlsym(LOG_NEVER, "%p\n", (void*)start);
            return (void*)start;
        }
        if(!dl->last_error)
            dl->last_error = box_malloc(129);
        snprintf(dl->last_error, 129, "Symbol \"%s\" version %s not found in %p)\n", rsymbol, vername?vername:"(nil)", handle);
        printf_dlsym(LOG_NEVER, "%p\n", NULL);
        return NULL;
    }
    if(handle==(void*)0xFFFFFFFF) {
        // special case, look globably but no self (RTLD_NEXT)
        elfheader_t *elf = FindElfAddress(my_context, *(uint32_t*)R_ESP); // use return address to guess "self"
        if(GetNoSelfSymbolStartEnd(my_context->maplib, rsymbol, &start, &end, elf, 0, version, vername)) {
            printf_dlsym(LOG_NEVER, "%p\n", (void*)start);
            return (void*)start;
        }
        if(!dl->last_error)
            dl->last_error = box_malloc(129);
        snprintf(dl->last_error, 129, "Symbol \"%s\" version %s not found in %p)\n", rsymbol, vername?vername:"(nil)", handle);
        printf_dlsym(LOG_NEVER, "%p\n", NULL);
        return NULL;
    }
    int nlib = (int)handle;
    --nlib;
    if(nlib<0 || nlib>=dl->lib_sz) {
        if(!dl->last_error)
            dl->last_error = box_malloc(129);
        snprintf(dl->last_error, 129, "Bad handle %p)\n", handle);
        printf_dlsym(LOG_NEVER, "%p\n", NULL);
        return NULL;
    }
    if(dl->count[nlib]==0) {
        if(!dl->last_error)
            dl->last_error = box_malloc(129);
        snprintf(dl->last_error, 129, "Bad handle %p (already closed))\n", handle);
        printf_dlsym(LOG_NEVER, "%p\n", (void*)NULL);
        return NULL;
    }
    if(dl->libs[nlib]) {
        if(my_dlsym_lib(dl->libs[nlib], rsymbol, &start, &end, version, vername)==0) {
            // not found
            printf_dlsym(LOG_NEVER, "%p\nCall to dlvsym(%s, \"%s\", %s) Symbol not found\n", NULL, GetNameLib(dl->libs[nlib]), rsymbol, vername?vername:"(nil)");
            if(!dl->last_error)
                dl->last_error = box_malloc(129);
            snprintf(dl->last_error, 129, "Symbol \"%s\" not found in %p(%s)", rsymbol, handle, GetNameLib(dl->libs[nlib]));
            return NULL;
        }
    } else {
        // still usefull?
        const char* defver = GetDefaultVersion(my_context->globaldefver, rsymbol);
        if(!defver) defver = GetDefaultVersion(my_context->weakdefver, rsymbol);
        if(GetGlobalSymbolStartEnd(my_context->maplib, rsymbol, &start, &end, NULL, -1, defver)) {
            printf_dlsym(LOG_NEVER, "%p\n", (void*)start);
            return (void*)start;
        }
        // not found
        printf_dlsym(LOG_NEVER, "%p\nCall to dlvsym(%s, \"%s\", %s) Symbol not found\n", NULL, "Self", rsymbol, vername?vername:"(nil)");
        if(!dl->last_error)
            dl->last_error = box_malloc(129);
        snprintf(dl->last_error, 129, "Symbol \"%s\" version %s not found in %p)\n", rsymbol, vername?vername:"(nil)", handle);
        return NULL;
    }
    printf_dlsym(LOG_NEVER, "%p\n", (void*)start);
    return (void*)start;
}

typedef struct link_map_s {
    uintptr_t   l_addr;
    char*       l_name;
    Elf32_Dyn*  l_ld;
} link_map_t;

int my_dlinfo(x86emu_t* emu, void* handle, int request, void* info)
{
        printf_dlsym(LOG_DEBUG, "Call to dlinfo(%p, %d, %p)\n", handle, request, info);
    dlprivate_t *dl = my_context->dlprivate;
    CLEARERR
    int nlib = (int)handle;
    --nlib;
    if(nlib<0 || nlib>=dl->lib_sz) {
        if(!dl->last_error)
            dl->last_error = box_malloc(129);
        snprintf(dl->last_error, 129, "Bad handle %p)\n", handle);
        printf_dlsym(LOG_DEBUG, "dlinfo: %s\n", dl->last_error);
        return -1;
    }
    if(dl->count[nlib]==0) {
        if(!dl->last_error)
            dl->last_error = box_malloc(129);
        snprintf(dl->last_error, 129, "Bad handle %p (already closed))\n", handle);
        printf_dlsym(LOG_DEBUG, "dlinfo: %s\n", dl->last_error);
        return -1;
    }
    library_t *lib = dl->libs[nlib];
    elfheader_t *h = (GetElfIndex(lib)>-1)?my_context->elfs[GetElfIndex(lib)]:NULL;
    switch(request) {
        case 2: // RTLD_DI_LINKMAP
            {
                static link_map_t map = {0};   //cheating, creating a structure on demand...
                *(link_map_t**)info = &map;
                map.l_addr = h?h->delta:0;
                map.l_name = lib->path;
                map.l_ld = h?h->Dynamic:NULL;
            }
            return 0;
        default:
            printf_log(LOG_NONE, "Warning, unsupported call to dlinfo(%p, %d, %p)\n", handle, request, info);
        if(!dl->last_error)
            dl->last_error = box_malloc(129);
        snprintf(dl->last_error, 129, "unsupported call to dlinfo request:%d\n", request);
    }
    return -1;
}

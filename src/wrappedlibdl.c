#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "library_private.h"
#include "x86emu_private.h"
#include "library.h"
#include "librarian.h"
#include "box86context.h"

typedef struct dlprivate_s {
    library_t   **libs;
    int         lib_sz;
    int         lib_cap;
    char*       last_error;
} dlprivate_t;

dlprivate_t *NewDLPrivate() {
    dlprivate_t* dl =  (dlprivate_t*)calloc(1, sizeof(dlprivate_t));
    return dl;
}
void FreeDLPrivate(dlprivate_t **lib) {
    free((*lib)->last_error);
    free(*lib);
}

void* my_dlopen(x86emu_t* emu, void *filename, int flag) EXPORT;
void* my_dlmopen(x86emu_t* emu, void* mlid, void *filename, int flag) EXPORT;
char* my_dlerror(x86emu_t* emu) EXPORT;
void* my_dlsym(x86emu_t* emu, void *handle, void *symbol) EXPORT;
int my_dlclose(x86emu_t* emu, void *handle) EXPORT;
int my_dladdr(x86emu_t* emu, void *addr, void *info) EXPORT;
void* my_dlvsym(x86emu_t* emu, void *handle, void *symbol, void *version) EXPORT;

#define LIBNAME libdl
const char* libdlName = "libdl.so.2";

// define all standard library functions
#include "wrappedlib_init.h"

#define CLEARERR    if(dl->last_error) free(dl->last_error); dl->last_error = NULL;

// Implementation
void* my_dlopen(x86emu_t* emu, void *filename, int flag)
{
    //void *dlopen(const char *filename, int flag);
    // TODO, handling special values for filename, like RTLD_SELF?
    // TODO, handling flags?
    library_t *lib = NULL;
    dlprivate_t *dl = emu->context->dlprivate;
    CLEARERR
    if(filename) {
        char* rfilename = (char*)filename;
        if(dlsym_error && box86_log<LOG_DEBUG) {
            printf_log(LOG_NONE, "Call to dlopen(\"%s\"/%p, %X)\n", rfilename, filename, flag);
        }
        printf_log(LOG_DEBUG, "Call to dlopen(\"%s\"/%p, %X)\n", rfilename, filename, flag);
        // check if alread dlopenned...
        for (int i=0; i<dl->lib_sz; ++i) {
            if(IsSameLib(dl->libs[i], rfilename))
                return (void*)(i+1);
        }
        // Then open the lib
        if(AddNeededLib(emu->context->maplib, rfilename, emu->context, emu)) {
            printf_log(LOG_INFO, "Warning: Cannot dlopen(\"%s\"/%p, %X)\n", rfilename, filename, flag);
            dl->last_error = malloc(129);
            snprintf(dl->last_error, 129, "Cannot dlopen(\"%s\"/%p, %X)\n", rfilename, filename, flag);
            return NULL;
        }
        if(FinalizeNeededLib(emu->context->maplib, rfilename, emu->context, emu)) {
            printf_log(LOG_INFO, "Warning: Cannot dlopen(\"%s\"/%p, %X)\n", rfilename, filename, flag);
            dl->last_error = malloc(129);
            snprintf(dl->last_error, 129, "Cannot dlopen(\"%s\"/%p, %X)\n", rfilename, filename, flag);
            return NULL;
        }
        lib = GetLib(emu->context->maplib, rfilename);
    } else {
        // check if alread dlopenned...
        for (int i=0; i<dl->lib_sz; ++i) {
            if(!dl->libs[i])
                return (void*)(i+1);
        }
        if(dlsym_error && box86_log<LOG_DEBUG) {
            printf_log(LOG_NONE, "Call to dlopen(NULL, %X)\n", flag);
        }
        printf_log(LOG_DEBUG, "Call to dlopen(NULL, %X)\n", flag);
    }
    //get the lib and add it to the collection
    
    if(dl->lib_sz == dl->lib_cap) {
        dl->lib_cap += 4;
        dl->libs = (library_t**)realloc(dl->libs, sizeof(library_t*)*dl->lib_cap);
    }
    dl->libs[dl->lib_sz] = lib;
    return (void*)(++dl->lib_sz);
}
void* my_dlmopen(x86emu_t* emu, void* lmid, void *filename, int flag)
{
    // lmid is ignored for now...
    return my_dlopen(emu, filename, flag);
}
char* my_dlerror(x86emu_t* emu)
{
    dlprivate_t *dl = emu->context->dlprivate;
    return dl->last_error;
}
void* my_dlsym(x86emu_t* emu, void *handle, void *symbol)
{
    dlprivate_t *dl = emu->context->dlprivate;
    uintptr_t start, end;
    char* rsymbol = (char*)symbol;
    CLEARERR
    printf_log(LOG_DEBUG, "Call to dlsym(%p, \"%s\")\n", handle, rsymbol);
    if(handle==NULL) {
        // special case, look globably
        if(GetGlobalSymbolStartEnd(emu->context->maplib, rsymbol, &start, &end))
            return (void*)start;
        dl->last_error = malloc(129);
        snprintf(dl->last_error, 129, "Symbol \"%s\" not found in %p)\n", rsymbol, handle);
        return NULL;
    }
    int nlib = (int)handle;
    --nlib;
    if(nlib<0 || nlib>=dl->lib_sz) {
        dl->last_error = malloc(129);
        snprintf(dl->last_error, 129, "Bad handle %p)\n", handle);
        return NULL;
    }
    if(dl->libs[nlib]) {
        if(dl->libs[nlib]->get(dl->libs[nlib], rsymbol, &start, &end)==0) {
            // not found
            if(dlsym_error && box86_log<LOG_DEBUG) {
                printf_log(LOG_NONE, "Call to dlsym(%s, \"%s\") Symbol not found\n", GetNameLib(dl->libs[nlib]), rsymbol);
            }
            printf_log(LOG_DEBUG, " Symbol not found\n");
            dl->last_error = malloc(129);
            snprintf(dl->last_error, 129, "Symbol \"%s\" not found in %p)\n", rsymbol, handle);
            return NULL;
        }
    } else {
        if(GetSymbolStartEnd(GetLocalSymbol(emu->context->maplib), rsymbol, &start, &end))
            return (void*)start;
        if(GetSymbolStartEnd(GetWeakSymbol(emu->context->maplib), rsymbol, &start, &end))
            return (void*)start;
        if(GetSymbolStartEnd(GetMapSymbol(emu->context->maplib), rsymbol, &start, &end))
            return (void*)start;
        // not found
        if(dlsym_error && box86_log<LOG_DEBUG) {
            printf_log(LOG_NONE, "Call to dlsym(%s, \"%s\") Symbol not found\n", "Self", rsymbol);
        }
        printf_log(LOG_DEBUG, " Symbol not found\n");
        dl->last_error = malloc(129);
        snprintf(dl->last_error, 129, "Symbol \"%s\" not found in %p)\n", rsymbol, handle);
        return NULL;
    }
    return (void*)start;
}
int my_dlclose(x86emu_t* emu, void *handle)
{
    printf_log(LOG_DEBUG, "Call to dlclose(%p)\n", handle);
    dlprivate_t *dl = emu->context->dlprivate;
    CLEARERR
    int nlib = (int)handle;
    --nlib;
    if(nlib<0 || nlib>=dl->lib_sz) {
        dl->last_error = malloc(129);
        snprintf(dl->last_error, 129, "Bad handle %p)\n", handle);
        return -1;
    }
    return 0;
}
int my_dladdr(x86emu_t* emu, void *addr, void *i)
{
    //int dladdr(void *addr, Dl_info *info);
    dlprivate_t *dl = emu->context->dlprivate;
    CLEARERR
    Dl_info *info = (Dl_info*)i;
    printf_log(LOG_INFO, "Warning: partially unimplement call to dladdr(%p, %p)\n", addr, info);
    
    //emu->quit = 1;
    info->dli_saddr = NULL;
    info->dli_sname = FindSymbolName(emu->context->maplib, addr, &info->dli_saddr, NULL, &info->dli_fname, &info->dli_fbase);
    return (info->dli_sname)?0:-1;
}
void* my_dlvsym(x86emu_t* emu, void *handle, void *symbol, void *version)
{
    dlprivate_t *dl = emu->context->dlprivate;
    CLEARERR
    //void *dlvsym(void *handle, char *symbol, char *version);
    char* rsymbol = (char*)symbol;
    char* rversion = (char*)version;
    printf_log(LOG_INFO, "Error: unimplement call to dlvsym(%p, %s, %s)\n", handle, rsymbol, rversion);
    emu->quit = 1;
    return NULL;
}

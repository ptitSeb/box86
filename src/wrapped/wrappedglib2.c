#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x86emu.h"
#include "emu/x86emu_private.h"
#include "callback.h"
#include "librarian.h"
#include "box86context.h"
#include "emu/x86emu_private.h"
#include "myalign.h"

const char* glib2Name = "libglib-2.0.so.0";
#define LIBNAME glib2

static box86context_t* my_context = NULL;

typedef void* (*pFp_t)(void*);
typedef void  (*vFpp_t)(void*, void*);
typedef void* (*pFpp_t)(void*, void*);
typedef uint32_t (*uFupp_t)(uint32_t, void*, void*);
typedef void* (*pFppp_t)(void*, void*, void*);
typedef void (*vFpppp_t)(void*, void*, void*, void*);
typedef void (*vFpupp_t)(void*, uint32_t, void*, void*);
typedef void* (*pFpupp_t)(void*, uint32_t, void*, void*);
typedef int (*iFpupppp_t)(void*, uint32_t, void*, void*, void*, void*);
typedef void* (*pFppuipp_t)(void*, void*, uint32_t, int32_t, void*, void*);

#define SUPER() \
    GO(g_list_free_full, vFpp_t)                \
    GO(g_markup_vprintf_escaped, pFpp_t)        \
    GO(g_build_filenamev, pFp_t)                \
    GO(g_timeout_add, uFupp_t)                  \
    GO(g_datalist_id_set_data_full, vFpupp_t)   \
    GO(g_datalist_id_dup_data, pFpupp_t)        \
    GO(g_datalist_id_replace_data, iFpupppp_t)  \
    GO(g_variant_new_from_data, pFppuipp_t)     \
    GO(g_variant_new_parsed_va, pFpp_t)         \
    GO(g_variant_get_va, vFpppp_t)              \
    GO(g_variant_new_va, pFppp_t)

typedef struct glib2_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} glib2_my_t;

void* getGlib2My(library_t* lib)
{
    glib2_my_t* my = (glib2_my_t*)calloc(1, sizeof(glib2_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeGlib2My(void* lib)
{
    glib2_my_t *my = (glib2_my_t *)lib;
}

x86emu_t* my_free_full_emu = NULL;
static void my_free_full_cb(void* data)
{
    if(!my_free_full_emu)
        return;
    SetCallbackArg(my_free_full_emu, 0, data);
    RunCallback(my_free_full_emu);
}
EXPORT void my_g_list_free_full(x86emu_t* emu, void* list, void* free_func)
{
    library_t * lib = GetLib(emu->context->maplib, glib2Name);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;
    x86emu_t *old = my_free_full_emu;
    my_free_full_emu = AddSharedCallback(emu, (uintptr_t)free_func, 1, NULL, NULL, NULL, NULL);
    my->g_list_free_full(list, my_free_full_cb);
    FreeCallback(my_free_full_emu);
    my_free_full_emu = old;
}

EXPORT void* my_g_markup_printf_escaped(x86emu_t *emu, void* fmt, void* b) {
    library_t * lib = GetLib(emu->context->maplib, glib2Name);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign((const char*)fmt, b, emu->scratch);
    return my->g_markup_vprintf_escaped(fmt, emu->scratch);
    #else
    // other platform don't need that
    return my->g_markup_vprintf_escaped(fmt, b);
    #endif
}

EXPORT void* my_g_markup_vprintf_escaped(x86emu_t *emu, void* fmt, void* b) {
    library_t * lib = GetLib(emu->context->maplib, glib2Name);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign((const char*)fmt, *(uint32_t**)b, emu->scratch);
    return my->g_markup_vprintf_escaped(fmt, emu->scratch);
    #else
    // other platform don't need that
    return my->g_markup_vprintf_escaped(fmt, *(uint32_t**)b);
    #endif
}

EXPORT void* my_g_build_filename(x86emu_t* emu, void* first, void** b)
{
    library_t * lib = GetLib(emu->context->maplib, glib2Name);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;
    int i = 0;
    while (b[i++]);
    void* array[i+1];   // +1 for 1st (NULL terminal already included)
    array[0] = first;
    memcpy(array+1, b, sizeof(void*)*i);
    void* ret = my->g_build_filenamev(array);
    return ret;
}

static int my_gsourcefunc(void* data)
{
    x86emu_t* emu = (x86emu_t*)data;
    int ret = RunCallback(emu);
    if(!ret)
        FreeCallback(emu);
    return ret;
}

EXPORT uint32_t my_g_timeout_add(x86emu_t* emu, uint32_t interval, void* func, void* data)
{
    library_t * lib = GetLib(emu->context->maplib, glib2Name);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;
    x86emu_t* cb = AddCallback(emu, (uintptr_t)func, 1, data, NULL, NULL, NULL);
    return my->g_timeout_add(interval, my_gsourcefunc, cb);
}

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)

#define GO(A)   \
static uintptr_t my_copy_fct_##A = 0;   \
static void* my_copy_##A(void* data)     \
{                                       \
    return (void*)RunFunction(my_context, my_copy_fct_##A, 1, data);\
}
SUPER()
#undef GO
static void* findCopyFct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my_copy_fct_##A == (uintptr_t)fct) return my_copy_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_copy_fct_##A == 0) {my_copy_fct_##A = (uintptr_t)fct; return my_copy_##A; }
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 Copy callback\n");
    return NULL;
}

#define GO(A)   \
static uintptr_t my_free_fct_##A = 0;   \
static void my_free_##A(void* data)     \
{                                       \
    RunFunction(my_context, my_free_fct_##A, 1, data);\
}
SUPER()
#undef GO
static void* findFreeFct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my_free_fct_##A == (uintptr_t)fct) return my_free_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_free_fct_##A == 0) {my_free_fct_##A = (uintptr_t)fct; return my_free_##A; }
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 Free callback\n");
    return NULL;
}
#undef SUPER

EXPORT void my_g_datalist_id_set_data_full(x86emu_t* emu, void* datalist, uint32_t key, void* data, void* freecb)
{
    library_t * lib = GetLib(emu->context->maplib, glib2Name);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;
    void* fc = findFreeFct(freecb);
    my->g_datalist_id_set_data_full(datalist, key, data, fc);
}

EXPORT void* my_g_datalist_id_dup_data(x86emu_t* emu, void* datalist, uint32_t key, void* dupcb, void* data)
{
    library_t * lib = GetLib(emu->context->maplib, glib2Name);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;
    void* cc = findFreeFct(dupcb);
    return my->g_datalist_id_dup_data(datalist, key, cc, data);
}

EXPORT int my_g_datalist_id_replace_data(x86emu_t* emu, void* datalist, uint32_t key, void* oldval, void* newval, void* oldfree, void* newfree)
{
    library_t * lib = GetLib(emu->context->maplib, glib2Name);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;
    void* oldfc = findFreeFct(oldfree);
    void* newfc = findFreeFct(newfree);
    return my->g_datalist_id_replace_data(datalist, key, oldval, newval, oldfc, newfc);
}

EXPORT void* my_g_variant_new_from_data(x86emu_t* emu, void* type, void* data, uint32_t size, int trusted, void* freecb, void* datacb)
{
    library_t * lib = GetLib(emu->context->maplib, glib2Name);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;
    void* fc = findFreeFct(freecb);
    return my->g_variant_new_from_data(type, data, size, trusted, fc, datacb);
}

EXPORT void* my_g_variant_new_parsed_va(x86emu_t* emu, void* fmt, void* b)
{
    library_t * lib = GetLib(emu->context->maplib, glib2Name);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    return my->g_variant_new_parsed_va(fmt, emu->scratch);
    #else
    return my->g_variant_new_parsed_va(fmt, b);
    #endif
}

EXPORT void my_g_variant_get(x86emu_t* emu, void* value, void* fmt, void* b)
{
    library_t * lib = GetLib(emu->context->maplib, glib2Name);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;
    my->g_variant_get_va(value, fmt, NULL, b);
}

EXPORT void* my_g_variant_new(x86emu_t* emu, void* fmt, void* b)
{
    library_t * lib = GetLib(emu->context->maplib, glib2Name);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;
    return my->g_variant_new_va(fmt, NULL, b);
}

#define CUSTOM_INIT \
    my_context = box86; \
    lib->priv.w.p2 = getGlib2My(lib);

#define CUSTOM_FINI \
    freeGlib2My(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"


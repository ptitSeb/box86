#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

typedef void (*vFp_t)(void*);
typedef void* (*pFp_t)(void*);
typedef void  (*vFpp_t)(void*, void*);
typedef int  (*iFpp_t)(void*, void*);
typedef void* (*pFpp_t)(void*, void*);
typedef void* (*pFpu_t)(void*, uint32_t);
typedef uint32_t  (*uFpp_t)(void*, void*);
typedef int  (*iFppp_t)(void*, void*, void*);
typedef uint32_t (*uFupp_t)(uint32_t, void*, void*);
typedef void* (*pFppp_t)(void*, void*, void*);
typedef void (*vFpppp_t)(void*, void*, void*, void*);
typedef void (*vFpupp_t)(void*, uint32_t, void*, void*);
typedef int (*iFpLpp_t)(void*, unsigned long, void*, void*);
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
    GO(g_variant_new_va, pFppp_t)               \
    GO(g_strdup_vprintf, pFpp_t)                \
    GO(g_vprintf, iFpp_t)                       \
    GO(g_vfprintf, iFppp_t)                     \
    GO(g_vsprintf, iFppp_t)                     \
    GO(g_vsnprintf, iFpLpp_t)                   \
    GO(g_vasprintf, iFppp_t)                    \
    GO(g_printf_string_upper_bound, uFpp_t)     \
    GO(g_source_new, pFpu_t)                    \
    GO(g_source_set_funcs, vFpp_t)              \
    GO(g_source_remove_by_funcs_user_data, iFpp_t) \
    GO(g_main_context_get_poll_func, pFp_t)     \
    GO(g_main_context_set_poll_func, vFpp_t)    \
    GO(g_print, vFp_t)                          \
    GO(g_printerr, vFp_t)

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
typedef int (*GSourceFunc) (void* user_data);

typedef struct my_GSourceFuncs_s {
  int  (*prepare)  (void* source, int* timeout_);
  int  (*check)    (void* source);
  int  (*dispatch) (void* source, GSourceFunc callback,void* user_data);
  void (*finalize) (void* source);
} my_GSourceFuncs_t;

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
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_copy_fct_##A == (uintptr_t)fct) return my_copy_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_copy_fct_##A == 0) {my_copy_fct_##A = (uintptr_t)fct; return my_copy_##A; }
    SUPER()
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
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_free_fct_##A == (uintptr_t)fct) return my_free_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_free_fct_##A == 0) {my_free_fct_##A = (uintptr_t)fct; return my_free_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 Free callback\n");
    return NULL;
}
// GSourceFuncs....
// g_source_new callback. First the structure GSourceFuncs statics, with paired x86 source pointer
#define GO(A) \
static my_GSourceFuncs_t     my_gsourcefuncs_##A = {0};   \
static my_GSourceFuncs_t   *ref_gsourcefuncs_##A = NULL;
SUPER()
#undef GO
// then the static functions callback that may be used with the structure, but dispatch also have a callback...
#define GO(A)   \
static uintptr_t fct_funcs_prepare_##A = 0;  \
static int my_funcs_prepare_##A(void* source, int timeout_) {   \
    return (int)RunFunction(my_context, fct_funcs_prepare_##A, 2, source, timeout_);    \
}   \
static uintptr_t fct_funcs_check_##A = 0;  \
static int my_funcs_check_##A(void* source) {   \
    return (int)RunFunction(my_context, fct_funcs_check_##A, 1, source);    \
}   \
static uintptr_t fct_funcs_dispatch_cb_##A = 0; \
static int my_funcs_dispatch_cb_##A(void* source) {   \
    return (int)RunFunction(my_context, fct_funcs_dispatch_cb_##A, 1);    \
}   \
static uintptr_t fct_funcs_dispatch_##A = 0;  \
static int my_funcs_dispatch_##A(void* source, void* cb, void* data) {   \
    uintptr_t old = fct_funcs_dispatch_cb_##A;  \
    fct_funcs_dispatch_cb_##A = (uintptr_t)cb;  \
    return (int)RunFunction(my_context, fct_funcs_dispatch_##A, 3, source, cb?my_funcs_dispatch_cb_##A:NULL, data);    \
    fct_funcs_dispatch_cb_##A = old;    \
}   \
static uintptr_t fct_funcs_finalize_##A = 0;  \
static int my_funcs_finalize_##A(void* source) {   \
    return (int)RunFunction(my_context, fct_funcs_finalize_##A, 1, source);    \
}
SUPER()
#undef GO
// and now the get slot / assign... Taking into account that the desired callback may already be a wrapped one (so unwrapping it)
static my_GSourceFuncs_t* findFreeGSourceFuncs(my_GSourceFuncs_t* fcts)
{
    if(!fcts) return fcts;
    #define GO(A) if(ref_gsourcefuncs_##A == fcts) return &my_gsourcefuncs_##A;
    SUPER()
    #undef GO
    #define GO(A) if(ref_gsourcefuncs_##A == 0) {   \
        ref_gsourcefuncs_##A = fcts;                 \
        my_gsourcefuncs_##A.prepare = (fcts->prepare)?((GetNativeFnc((uintptr_t)fcts->prepare))?GetNativeFnc((uintptr_t)fcts->prepare):my_funcs_prepare_##A):NULL;    \
        fct_funcs_prepare_##A = (uintptr_t)fcts->prepare;                            \
        my_gsourcefuncs_##A.check = (fcts->check)?((GetNativeFnc((uintptr_t)fcts->check))?GetNativeFnc((uintptr_t)fcts->check):my_funcs_check_##A):NULL;    \
        fct_funcs_check_##A = (uintptr_t)fcts->check;                            \
        my_gsourcefuncs_##A.dispatch = (fcts->dispatch)?((GetNativeFnc((uintptr_t)fcts->dispatch))?GetNativeFnc((uintptr_t)fcts->dispatch):my_funcs_dispatch_##A):NULL;    \
        fct_funcs_dispatch_##A = (uintptr_t)fcts->dispatch;                            \
        my_gsourcefuncs_##A.finalize = (fcts->finalize)?((GetNativeFnc((uintptr_t)fcts->finalize))?GetNativeFnc((uintptr_t)fcts->finalize):my_funcs_finalize_##A):NULL;    \
        fct_funcs_finalize_##A = (uintptr_t)fcts->finalize;                            \
        return &my_gsourcefuncs_##A;                \
    }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GSourceFuncs callback\n");
    return NULL;
}

// PollFunc ...
#define GO(A)   \
static uintptr_t my_poll_fct_##A = 0;   \
static int my_poll_##A(void* ufds, uint32_t nfsd, int32_t timeout_)     \
{                                       \
    return RunFunction(my_context, my_poll_fct_##A, 3, ufds, nfsd, timeout_);\
}
SUPER()
#undef GO
static void* findPollFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_poll_fct_##A == (uintptr_t)fct) return my_poll_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_poll_fct_##A == 0) {my_poll_fct_##A = (uintptr_t)fct; return my_poll_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 Poll callback\n");
    return NULL;
}

static void* reversePollFct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if((uintptr_t)fct == my_poll_fct_##A) return (void*)my_poll_fct_##A;
    SUPER()
    #undef GO
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

EXPORT void* my_g_strdup_vprintf(x86emu_t* emu, void* fmt, void* b)
{
    library_t * lib = GetLib(emu->context->maplib, glib2Name);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    return my->g_strdup_vprintf(fmt, emu->scratch);
    #else
    return my->g_strdup_vprintf(fmt, b);
    #endif
}

EXPORT int my_g_vprintf(x86emu_t* emu, void* fmt, void* b)
{
    library_t * lib = GetLib(emu->context->maplib, glib2Name);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    return my->g_vprintf(fmt, emu->scratch);
    #else
    return my->g_vprintf(fmt, b);
    #endif
}

EXPORT int my_g_vfprintf(x86emu_t* emu, void* F, void* fmt, void* b)
{
    library_t * lib = GetLib(emu->context->maplib, glib2Name);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    return my->g_vfprintf(F, fmt, emu->scratch);
    #else
    return my->g_vfprintf(F, fmt, b);
    #endif
}

EXPORT int my_g_vsprintf(x86emu_t* emu, void* s, void* fmt, void* b)
{
    library_t * lib = GetLib(emu->context->maplib, glib2Name);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    return my->g_vsprintf(s, fmt, emu->scratch);
    #else
    return my->g_vsprintf(s, fmt, b);
    #endif
}

EXPORT int my_g_vsnprintf(x86emu_t* emu, void* s, unsigned long n, void* fmt, void* b)
{
    library_t * lib = GetLib(emu->context->maplib, glib2Name);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    return my->g_vsnprintf(s, n, fmt, emu->scratch);
    #else
    return my->g_vsnprintf(s, n, fmt, b);
    #endif
}

EXPORT int my_g_vasprintf(x86emu_t* emu, void* s, void* fmt, void* b)
{
    library_t * lib = GetLib(emu->context->maplib, glib2Name);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    return my->g_vasprintf(s, fmt, emu->scratch);
    #else
    return my->g_vasprintf(s, fmt, b);
    #endif
}

EXPORT uint32_t my_g_printf_string_upper_bound(x86emu_t* emu, void* fmt, void* b)
{
    library_t * lib = GetLib(emu->context->maplib, glib2Name);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    return my->g_printf_string_upper_bound(fmt, emu->scratch);
    #else
    return my->g_printf_string_upper_bound(fmt, b);
    #endif
}

EXPORT void my_g_print(x86emu_t* emu, void* fmt, void* b)
{
    library_t * lib = GetLib(emu->context->maplib, glib2Name);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    char* buf = NULL;
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    iFppp_t f = (iFppp_t)vasprintf;
    f(&buf, fmt, emu->scratch);
    #else
    iFppp_t f = (iFppp_t)vasprintf;
    f(&buf, fmt, b);
    #endif
    my->g_print(buf);
    free(buf);
}

EXPORT void my_g_printerr(x86emu_t* emu, void* fmt, void* b)
{
    library_t * lib = GetLib(emu->context->maplib, glib2Name);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    char* buf = NULL;
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    iFppp_t f = (iFppp_t)vasprintf;
    f(&buf, fmt, emu->scratch);
    #else
    iFppp_t f = (iFppp_t)vasprintf;
    f(&buf, fmt, b);
    #endif
    my->g_printerr(buf);
    free(buf);
}

EXPORT void* my_g_source_new(x86emu_t* emu, my_GSourceFuncs_t* source_funcs, uint32_t struct_size)
{
    library_t * lib = GetLib(emu->context->maplib, glib2Name);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    return my->g_source_new(findFreeGSourceFuncs(source_funcs), struct_size);
}

EXPORT void my_g_source_set_funcs(x86emu_t* emu, void* source, my_GSourceFuncs_t* source_funcs)
{
    library_t * lib = GetLib(emu->context->maplib, glib2Name);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    my->g_source_set_funcs(source, findFreeGSourceFuncs(source_funcs));
}


EXPORT int my_g_source_remove_by_funcs_user_data(x86emu_t* emu, my_GSourceFuncs_t* source_funcs, void* data)
{
    library_t * lib = GetLib(emu->context->maplib, glib2Name);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    return my->g_source_remove_by_funcs_user_data(findFreeGSourceFuncs(source_funcs), data);
}

EXPORT void* my_g_main_context_get_poll_func(x86emu_t* emu, void* context)
{
    library_t * lib = GetLib(emu->context->maplib, glib2Name);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    void* ret = my->g_main_context_get_poll_func(context);
    if(!ret) return ret;
    void* r = reversePollFct(ret);
    if(r) return r;
    // needs to bridge....
    return (void*)AddCheckBridge(lib->priv.w.bridge, iFpui, r, 0);
}
    
EXPORT void my_g_main_context_set_poll_func(x86emu_t* emu, void* context, void* func)
{
    library_t * lib = GetLib(emu->context->maplib, glib2Name);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    my->g_main_context_set_poll_func(context, findPollFct(func));
}

#define CUSTOM_INIT \
    my_context = box86; \
    lib->priv.w.p2 = getGlib2My(lib);

#define CUSTOM_FINI \
    freeGlib2My(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"


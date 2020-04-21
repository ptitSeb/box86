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
#include "gtkclass.h"

const char* glib2Name = "libglib-2.0.so.0";
#define LIBNAME glib2

static char* libname = NULL;

typedef void (*vFp_t)(void*);
typedef void* (*pFp_t)(void*);
typedef void  (*vFpp_t)(void*, void*);
typedef void* (*pFup_t)(uint32_t, void*);
typedef int  (*iFpp_t)(void*, void*);
typedef void* (*pFpp_t)(void*, void*);
typedef void* (*pFpu_t)(void*, uint32_t);
typedef uint32_t  (*uFpp_t)(void*, void*);
typedef int  (*iFppp_t)(void*, void*, void*);
typedef uint32_t (*uFipp_t)(int, void*, void*);
typedef uint32_t  (*uFppp_t)(void*, void*, void*);
typedef void  (*vFppp_t)(void*, void*, void*);
typedef uint32_t (*uFupp_t)(uint32_t, void*, void*);
typedef void* (*pFppp_t)(void*, void*, void*);
typedef void* (*pFppip_t)(void*, void*, int, void*);
typedef uint32_t (*uFuppp_t)(uint32_t, void*, void*, void*);
typedef uint32_t (*uFippp_t)(int, void*, void*, void*);
typedef uint32_t (*uFiuppp_t)(int, uint32_t, void*, void*, void*);
typedef void (*vFpiLpp_t)(void*, int, unsigned long, void*, void*);
typedef void* (*pFpppp_t)(void*, void*, void*, void*);
typedef void (*vFpppp_t)(void*, void*, void*, void*);
typedef void (*vFpupp_t)(void*, uint32_t, void*, void*);
typedef int (*iFpLpp_t)(void*, unsigned long, void*, void*);
typedef void* (*pFpupp_t)(void*, uint32_t, void*, void*);
typedef uint32_t (*uFiippp_t)(int, int, void*, void*, void*);
typedef int (*iFpupppp_t)(void*, uint32_t, void*, void*, void*, void*);
typedef void* (*pFppuipp_t)(void*, void*, uint32_t, int32_t, void*, void*);
typedef void* (*pFppLiiip_t)(void*, void*, unsigned long, int, int, int, void*);
typedef int (*iFpppipppp_t)(void*, void*, void*, int, void*, void*, void*, void*);
typedef int (*iFpppipppppp_t)(void*, void*, void*, int, void*, void*, void*, void*, void*, void*);
typedef int (*iFpppippppppp_t)(void*, void*, void*, int, void*, void*, void*, void*, void*, void*, void*);

#define SUPER() \
    GO(g_list_free_full, vFpp_t)                \
    GO(g_markup_vprintf_escaped, pFpp_t)        \
    GO(g_build_filenamev, pFp_t)                \
    GO(g_timeout_add_full, uFiuppp_t)            \
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
    GO(g_printerr, vFp_t)                       \
    GO(g_idle_add_full, uFippp_t)               \
    GO(g_hash_table_new, pFpp_t)                \
    GO(g_hash_table_new_full, pFpppp_t)         \
    GO(g_hash_table_foreach, vFppp_t)           \
    GO(g_hash_table_foreach_remove, uFppp_t)    \
    GO(g_hash_table_foreach_steal, uFppp_t)     \
    GO(g_hash_table_find, pFppp_t)              \
    GO(g_spawn_async_with_pipes, iFpppippppppp_t)\
    GO(g_spawn_async, iFpppipppp_t)             \
    GO(g_spawn_sync, iFpppipppppp_t)            \
    GO(g_child_watch_add, uFipp_t)              \
    GO(g_child_watch_add_full, uFiippp_t)       \
    GO(g_private_new, pFp_t)                    \
    GO(g_static_private_set, vFppp_t)           \
    GO(g_ptr_array_new_with_free_func, pFp_t)   \
    GO(g_ptr_array_new_full, pFup_t)            \
    GO(g_ptr_array_set_free_func, vFpp_t)       \
    GO(g_ptr_array_sort_with_data, vFppp_t)     \
    GO(g_ptr_array_foreach, vFppp_t)            \
    GO(g_qsort_with_data, vFpiLpp_t)            \
    GO(g_thread_create, pFppip_t)               \
    GO(g_thread_create_full, pFppLiiip_t)       \
    GO(g_thread_foreach, vFpp_t)                \
    GO(g_array_sort, vFpp_t)                    \
    GO(g_array_sort_with_data, vFppp_t)         \
    GO(g_array_set_clear_func, vFpp_t)          \
    GO(g_source_set_callback, vFpppp_t)         \
    GO(g_slist_insert_sorted_with_data, pFpppp_t)   \
    GO(g_slist_foreach, pFppp_t)                \
    GO(g_slist_find_custom, pFppp_t)            \
    GO(g_idle_add, uFpp_t)

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
    //glib2_my_t *my = (glib2_my_t *)lib;
}

static void my_destroy_notify(void* data)   // data should be arg 8
{
    x86emu_t *emu = (x86emu_t*)data;
    uintptr_t f = (uintptr_t)GetCallbackArg(emu, 9);
    if(f) {
        SetCallbackNArg(emu, 1);
        SetCallbackArg(emu, 0, GetCallbackArg(emu, 8));
        SetCallbackAddress(emu, f);
        RunCallback(emu);
    }
    FreeCallback(emu);
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
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;
    x86emu_t *old = my_free_full_emu;
    my_free_full_emu = AddSharedCallback(emu, (uintptr_t)free_func, 1, NULL, NULL, NULL, NULL);
    my->g_list_free_full(list, my_free_full_cb);
    FreeCallback(my_free_full_emu);
    my_free_full_emu = old;
}

EXPORT void* my_g_markup_printf_escaped(x86emu_t *emu, void* fmt, void* b) {
    library_t * lib = GetLib(emu->context->maplib, libname);
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
    library_t * lib = GetLib(emu->context->maplib, libname);
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

EXPORT void* my_g_build_filename(x86emu_t* emu, void* first, void** b)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;
    int i = 0;
    while (b[i++]);
    void* array[i+1];   // +1 for 1st (NULL terminal already included)
    array[0] = first;
    memcpy(array+1, b, sizeof(void*)*i);
    void* ret = my->g_build_filenamev(array);
    return ret;
}

static int my_timeout_cb(my_signal_t* sig)
{
    return (int)RunFunction(my_context, sig->c_handler, 1, sig->data);
}
EXPORT uint32_t my_g_timeout_add(x86emu_t* emu, uint32_t interval, void* func, void* data)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    my_signal_t *sig = new_mysignal(func, data, NULL);
    return my->g_timeout_add_full(0, interval, my_timeout_cb, sig, my_signal_delete);
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

// GCopyFct
/*#define GO(A)   \
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
}*/
// GFreeFct
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
// GDuplicateFct
#define GO(A)   \
static uintptr_t my_duplicate_fct_##A = 0;   \
static void* my_duplicate_##A(void* data)     \
{                                       \
    return (void*)RunFunction(my_context, my_duplicate_fct_##A, 1, data);\
}
SUPER()
#undef GO
static void* findDuplicateFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_duplicate_fct_##A == (uintptr_t)fct) return my_duplicate_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_duplicate_fct_##A == 0) {my_duplicate_fct_##A = (uintptr_t)fct; return my_duplicate_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 Duplicate callback\n");
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
static int my_funcs_dispatch_cb_##A(void* a, void* b, void* c, void* d) {   \
    return (int)RunFunction(my_context, fct_funcs_dispatch_cb_##A, 4, a, b, c, d);    \
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

// GHashFunc ...
#define GO(A)   \
static uintptr_t my_hashfunc_fct_##A = 0;   \
static uint32_t my_hashfunc_##A(void* key)     \
{                                       \
    return (uint32_t)RunFunction(my_context, my_hashfunc_fct_##A, 1, key);\
}
SUPER()
#undef GO
static void* findHashFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_hashfunc_fct_##A == (uintptr_t)fct) return my_hashfunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_hashfunc_fct_##A == 0) {my_hashfunc_fct_##A = (uintptr_t)fct; return my_hashfunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GHashFunc callback\n");
    return NULL;
}
// GEqualFunc ...
#define GO(A)   \
static uintptr_t my_equalfunc_fct_##A = 0;   \
static int my_equalfunc_##A(void* a, void* b)     \
{                                       \
    return RunFunction(my_context, my_equalfunc_fct_##A, 2, a, b);\
}
SUPER()
#undef GO
static void* findEqualFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_equalfunc_fct_##A == (uintptr_t)fct) return my_equalfunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_equalfunc_fct_##A == 0) {my_equalfunc_fct_##A = (uintptr_t)fct; return my_equalfunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GEqualFunc callback\n");
    return NULL;
}
// GDestroyFunc ...
#define GO(A)   \
static uintptr_t my_destroyfunc_fct_##A = 0;   \
static int my_destroyfunc_##A(void* a, void* b)     \
{                                       \
    return RunFunction(my_context, my_destroyfunc_fct_##A, 2, a, b);\
}
SUPER()
#undef GO
static void* findDestroyFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_destroyfunc_fct_##A == (uintptr_t)fct) return my_destroyfunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_destroyfunc_fct_##A == 0) {my_destroyfunc_fct_##A = (uintptr_t)fct; return my_destroyfunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GDestroyNotify callback\n");
    return NULL;
}
// GSpawnChildSetupFunc ...
#define GO(A)   \
static uintptr_t my_spwnchildsetup_fct_##A = 0;   \
static void my_spwnchildsetup_##A(void* data)     \
{                                       \
    RunFunction(my_context, my_spwnchildsetup_fct_##A, 1, data);\
}
SUPER()
#undef GO
static void* findSpawnChildSetupFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_spwnchildsetup_fct_##A == (uintptr_t)fct) return my_spwnchildsetup_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_spwnchildsetup_fct_##A == 0) {my_spwnchildsetup_fct_##A = (uintptr_t)fct; return my_spwnchildsetup_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GSpawnChildSetup callback\n");
    return NULL;
}
// GSourceFunc ...
#define GO(A)   \
static uintptr_t my_GSourceFunc_fct_##A = 0;   \
static void my_GSourceFunc_##A(void* a, void* b, void* c, void* d)     \
{                                       \
    RunFunction(my_context, my_GSourceFunc_fct_##A, 4, a, b, c, d);\
}
SUPER()
#undef GO
static void* findGSourceFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GSourceFunc_fct_##A == (uintptr_t)fct) return my_GSourceFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GSourceFunc_fct_##A == 0) {my_GSourceFunc_fct_##A = (uintptr_t)fct; return my_GSourceFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GSourceFunc callback\n");
    return NULL;
}
// GCompareFunc ...
#define GO(A)   \
static uintptr_t my_GCompareFunc_fct_##A = 0;   \
static int my_GCompareFunc_##A(void* a, void* b)     \
{                                       \
    return (int)RunFunction(my_context, my_GCompareFunc_fct_##A, 2, a, b);\
}
SUPER()
#undef GO
static void* findGCompareFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GCompareFunc_fct_##A == (uintptr_t)fct) return my_GCompareFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GCompareFunc_fct_##A == 0) {my_GCompareFunc_fct_##A = (uintptr_t)fct; return my_GCompareFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GCompareFunc callback\n");
    return NULL;
}
#undef SUPER

EXPORT void my_g_datalist_id_set_data_full(x86emu_t* emu, void* datalist, uint32_t key, void* data, void* freecb)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;
    void* fc = findFreeFct(freecb);
    my->g_datalist_id_set_data_full(datalist, key, data, fc);
}

EXPORT void* my_g_datalist_id_dup_data(x86emu_t* emu, void* datalist, uint32_t key, void* dupcb, void* data)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;
    void* cc = findDuplicateFct(dupcb);
    return my->g_datalist_id_dup_data(datalist, key, cc, data);
}

EXPORT int my_g_datalist_id_replace_data(x86emu_t* emu, void* datalist, uint32_t key, void* oldval, void* newval, void* oldfree, void* newfree)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;
    void* oldfc = findFreeFct(oldfree);
    void* newfc = findFreeFct(newfree);
    return my->g_datalist_id_replace_data(datalist, key, oldval, newval, oldfc, newfc);
}

EXPORT void* my_g_variant_new_from_data(x86emu_t* emu, void* type, void* data, uint32_t size, int trusted, void* freecb, void* datacb)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;
    void* fc = findFreeFct(freecb);
    return my->g_variant_new_from_data(type, data, size, trusted, fc, datacb);
}

EXPORT void* my_g_variant_new_parsed_va(x86emu_t* emu, void* fmt, void* b)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
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
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;
    my->g_variant_get_va(value, fmt, NULL, b);
}

EXPORT void* my_g_variant_new(x86emu_t* emu, void* fmt, void* b)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;
    return my->g_variant_new_va(fmt, NULL, b);
}

EXPORT void* my_g_strdup_vprintf(x86emu_t* emu, void* fmt, void* b)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
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
    library_t * lib = GetLib(emu->context->maplib, libname);
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
    library_t * lib = GetLib(emu->context->maplib, libname);
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
    library_t * lib = GetLib(emu->context->maplib, libname);
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
    library_t * lib = GetLib(emu->context->maplib, libname);
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
    library_t * lib = GetLib(emu->context->maplib, libname);
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
    library_t * lib = GetLib(emu->context->maplib, libname);
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
    library_t * lib = GetLib(emu->context->maplib, libname);
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
    library_t * lib = GetLib(emu->context->maplib, libname);
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
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    return my->g_source_new(findFreeGSourceFuncs(source_funcs), struct_size);
}

EXPORT void my_g_source_set_funcs(x86emu_t* emu, void* source, my_GSourceFuncs_t* source_funcs)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    my->g_source_set_funcs(source, findFreeGSourceFuncs(source_funcs));
}


EXPORT int my_g_source_remove_by_funcs_user_data(x86emu_t* emu, my_GSourceFuncs_t* source_funcs, void* data)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    return my->g_source_remove_by_funcs_user_data(findFreeGSourceFuncs(source_funcs), data);
}

EXPORT void* my_g_main_context_get_poll_func(x86emu_t* emu, void* context)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    void* ret = my->g_main_context_get_poll_func(context);
    if(!ret) return ret;
    void* r = reversePollFct(ret);
    if(r) return r;
    // needs to bridge....
    return (void*)AddCheckBridge(lib->priv.w.bridge, iFpui, ret, 0);
}
    
EXPORT void my_g_main_context_set_poll_func(x86emu_t* emu, void* context, void* func)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    my->g_main_context_set_poll_func(context, findPollFct(func));
}

EXPORT uint32_t my_g_idle_add_full(x86emu_t* emu, int priority, void* f, void* data, void* notify)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    if(!f)
        return my->g_idle_add_full(priority, f, data, notify);

    my_signal_t *sig = new_mysignal(f, data, notify);
    printf_log(LOG_DEBUG, "glib2 Idle CB with priority %d created for %p, sig=%p\n", priority, f, sig);
    return my->g_idle_add_full(priority, my_timeout_cb, sig, my_signal_delete);
}

EXPORT void* my_g_hash_table_new(x86emu_t* emu, void* hash, void* equal)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    return my->g_hash_table_new(findHashFct(hash), findEqualFct(equal));
}

EXPORT void* my_g_hash_table_new_full(x86emu_t* emu, void* hash, void* equal, void* destroy_key, void* destroy_val)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    return my->g_hash_table_new_full(findHashFct(hash), findEqualFct(equal), findDestroyFct(destroy_key), findDestroyFct(destroy_val));
}

static void my_ghfunc(void* key, void* value, x86emu_t* emu)
{
    SetCallbackArgs(emu, 2, key, value);
    RunCallback(emu);
}
EXPORT void my_g_hash_table_foreach(x86emu_t* emu, void* table, void* f, void* data)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    x86emu_t* cb = AddSharedCallback(emu, (uintptr_t)f, 3, NULL, NULL, data, NULL);
    my->g_hash_table_foreach(table, my_ghfunc, cb);
    FreeCallback(cb);
}

static int my_ghrfunc(void* key, void* value, x86emu_t* emu)
{
    SetCallbackArgs(emu, 2, key, value);
    return RunCallback(emu);
}
EXPORT uint32_t my_g_hash_table_foreach_remove(x86emu_t* emu, void* table, void* f, void* data)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;
    
    x86emu_t* cb = AddSharedCallback(emu, (uintptr_t)f, 3, NULL, NULL, data, NULL);
    uint32_t ret = my->g_hash_table_foreach_remove(table, my_ghrfunc, cb);
    FreeCallback(cb);
    return ret;
}
EXPORT uint32_t my_g_hash_table_foreach_steal(x86emu_t* emu, void* table, void* f, void* data)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;
    
    x86emu_t* cb = AddSharedCallback(emu, (uintptr_t)f, 3, NULL, NULL, data, NULL);
    uint32_t ret = my->g_hash_table_foreach_steal(table, my_ghrfunc, cb);
    FreeCallback(cb);
    return ret;
}
EXPORT void* my_g_hash_table_find(x86emu_t* emu, void* table, void* f, void* data)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;
    
    x86emu_t* cb = AddSharedCallback(emu, (uintptr_t)f, 3, NULL, NULL, data, NULL);
    void* ret = my->g_hash_table_find(table, my_ghrfunc, cb);
    FreeCallback(cb);
    return ret;
}

EXPORT int my_g_spawn_async_with_pipes(x86emu_t* emu, void* dir, void* argv, void* envp, int flags, void* f, void* data, void* child, void* input, void* output, void* err, void* error)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    return my->g_spawn_async_with_pipes(dir, argv, envp, flags, findSpawnChildSetupFct(f), data, child, input, output, err, error);
}

EXPORT int my_g_spawn_async(x86emu_t* emu, void* dir, void* argv, void* envp, int flags, void* f, void* data, void* child, void* error)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    return my->g_spawn_async(dir, argv, envp, flags, findSpawnChildSetupFct(f), data, child, error);
}

EXPORT int my_g_spawn_sync(x86emu_t* emu, void* dir, void* argv, void* envp, int flags, void* f, void* data, void* input, void* output, void* status, void* error)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    return my->g_spawn_sync(dir, argv, envp, flags, findSpawnChildSetupFct(f), data, input, output, status, error);
}

static void my_gchildwatchfunc(int pid, int status, x86emu_t* emu)
{
    SetCallbackArgs(emu, 2, pid, status);
    RunCallback(emu);
}
EXPORT uint32_t my_g_child_watch_add(x86emu_t* emu, int pid, void* f, void* data)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    if(!f)
        return my->g_child_watch_add(pid, f, data);

    x86emu_t* cb = AddCallback(emu, (uintptr_t)f, 3, NULL, NULL, data, NULL);
    SetCallbackArg(cb, 9, NULL);
    SetCallbackArg(cb, 8, data);
    return my->g_child_watch_add_full(0, pid, my_gchildwatchfunc, cb, my_destroy_notify);
}

EXPORT uint32_t my_g_child_watch_add_full(x86emu_t* emu, int priority, int pid, void* f, void* data, void* notify)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    x86emu_t* cb = AddCallback(emu, (uintptr_t)f, 3, NULL, NULL, data, NULL);
    SetCallbackArg(cb, 9, notify);
    SetCallbackArg(cb, 8, data);
    return my->g_idle_add_full(priority, f?my_gchildwatchfunc:NULL, cb, my_destroy_notify);
}

EXPORT void* my_g_private_new(x86emu_t* emu, void* notify)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    return my->g_private_new(findFreeFct(notify));
}

EXPORT void my_g_static_private_set(x86emu_t* emu, void* private, void* data, void* notify)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    my->g_static_private_set(private, data, findFreeFct(notify));
}

EXPORT void* my_g_ptr_array_new_with_free_func(x86emu_t* emu, void* notify)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    return my->g_ptr_array_new_with_free_func(findFreeFct(notify));
}

EXPORT void* my_g_ptr_array_new_full(x86emu_t* emu, uint32_t size, void* notify)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    return my->g_ptr_array_new_full(size, findFreeFct(notify));
}

EXPORT void my_g_ptr_array_set_free_func(x86emu_t* emu, void* array, void* notify)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    my->g_ptr_array_set_free_func(array, findFreeFct(notify));
}

static int my_compare_fnc(void* a, void* b, x86emu_t* emu)
{
    SetCallbackArgs(emu, 2, a, b);
    return RunCallback(emu);
}
EXPORT void my_g_ptr_array_sort(x86emu_t* emu, void* array, void* comp)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    x86emu_t* emucb = AddSharedCallback(emu, (uintptr_t)comp, 2, NULL, NULL, NULL, NULL);
    my->g_ptr_array_sort_with_data(array, my_compare_fnc, emucb);
    FreeCallback(emucb);
}

EXPORT void my_g_ptr_array_sort_with_data(x86emu_t* emu, void* array, void* comp, void* data)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    x86emu_t* emucb = AddSharedCallback(emu, (uintptr_t)comp, 3, NULL, NULL, data, NULL);
    my->g_ptr_array_sort_with_data(array, my_compare_fnc, emucb);
    FreeCallback(emucb);
}

EXPORT void my_g_qsort_with_data(x86emu_t* emu, void* pbase, int total, unsigned long size, void* comp, void* data)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    x86emu_t* emucb = AddSharedCallback(emu, (uintptr_t)comp, 3, NULL, NULL, data, NULL);
    my->g_qsort_with_data(pbase, total, size, my_compare_fnc, emucb);
    FreeCallback(emucb);
}

static void my_g_foreach_fnc(void* a, x86emu_t* emu)
{
    SetCallbackArg(emu, 0, a);
    RunCallback(emu);
}
EXPORT void my_g_ptr_array_foreach(x86emu_t* emu, void* array, void* func, void* data)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    x86emu_t* emucb = AddSharedCallback(emu, (uintptr_t)func, 2, NULL, data, NULL, NULL);
    my->g_ptr_array_foreach(array, my_g_foreach_fnc, emucb);
    FreeCallback(emucb);
}

static void* my_g_thread_create_fnc(x86emu_t* emu)
{
    void* ret = (void*)RunCallback(emu);
    FreeCallback(emu);  // memory leak if thread is killed...
    return ret;
}
EXPORT void* my_g_thread_create(x86emu_t* emu, void* func, void* data, int joinable, void* error)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    x86emu_t* emucb = AddCallback(emu, (uintptr_t)func, 1, data, NULL, NULL, NULL);
    void* ret = my->g_thread_create(my_g_thread_create_fnc, emucb, joinable, error);
    if(!ret)
        FreeCallback(emucb);
    return ret;
}

EXPORT void* my_g_thread_create_full(x86emu_t* emu, void* func, void* data, unsigned long stack, int joinable, int bound, int priority, void* error)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    x86emu_t* emucb = AddVariableCallback(emu, stack, (uintptr_t)func, 1, data, NULL, NULL, NULL);
    void* ret = my->g_thread_create_full(my_g_thread_create_fnc, emucb, stack, joinable, bound, priority, error);
    if(!ret)
        FreeCallback(emucb);
    return ret;
}

EXPORT void my_g_thread_foreach(x86emu_t* emu, void* func, void* data)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    x86emu_t* emucb = AddSharedCallback(emu, (uintptr_t)func, 2, NULL, data, NULL, NULL);
    my->g_thread_foreach(my_g_foreach_fnc, emucb);
    FreeCallback(emucb);
}

EXPORT void my_g_array_sort(x86emu_t* emu, void* array, void* comp)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    x86emu_t* emucb = AddSharedCallback(emu, (uintptr_t)comp, 2, NULL, NULL, NULL, NULL);
    my->g_array_sort_with_data(array, my_compare_fnc, emucb);
    FreeCallback(emucb);
}

EXPORT void my_g_array_sort_with_data(x86emu_t* emu, void* array, void* comp, void* data)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    x86emu_t* emucb = AddSharedCallback(emu, (uintptr_t)comp, 3, NULL, NULL, data, NULL);
    my->g_array_sort_with_data(array, my_compare_fnc, emucb);
    FreeCallback(emucb);
}

EXPORT void my_g_array_set_clear_func(x86emu_t* emu, void* array, void* notify)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    my->g_array_set_clear_func(array, findFreeFct(notify));
}

EXPORT void my_g_source_set_callback(x86emu_t* emu, void* source, void* func, void* data, void* notify)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    my->g_source_set_callback(source, findGSourceFuncFct(func), data, findFreeFct(notify));
}

EXPORT void* my_g_slist_insert_sorted(x86emu_t* emu, void* list, void* d, void* comp)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    x86emu_t* emucb = AddSharedCallback(emu, (uintptr_t)comp, 2, NULL, NULL, NULL, NULL);
    void* ret = my->g_slist_insert_sorted_with_data(list, d, my_compare_fnc, emucb);
    FreeCallback(emucb);
    return ret;
}
EXPORT void* my_g_slist_insert_sorted_with_data(x86emu_t* emu, void* list, void* d, void* comp, void* data)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    x86emu_t* emucb = AddSharedCallback(emu, (uintptr_t)comp, 3, NULL, NULL, data, NULL);
    void* ret = my->g_slist_insert_sorted_with_data(list, d, my_compare_fnc, emucb);
    FreeCallback(emucb);
    return ret;
}

EXPORT void my_g_slist_foreach(x86emu_t* emu, void* list, void* func, void* data)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    x86emu_t* emucb = AddSharedCallback(emu, (uintptr_t)func, 2, NULL, data, NULL, NULL);
    my->g_slist_foreach(list, my_g_foreach_fnc, emucb);
    FreeCallback(emucb);
}

EXPORT void* my_g_slist_find_custom(x86emu_t* emu, void* list, void* data, void* comp)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    return my->g_slist_find_custom(list, data, findGCompareFuncFct(comp));
}

EXPORT uint32_t my_g_idle_add(x86emu_t* emu, void* func, void* data)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;

    return my->g_idle_add(findGSourceFuncFct(func), data);
}

#define CUSTOM_INIT \
    libname = lib->name;\
    lib->priv.w.p2 = getGlib2My(lib);

#define CUSTOM_FINI \
    freeGlib2My(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"


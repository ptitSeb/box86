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
#include "threads.h"

const char* glib2Name = "libglib-2.0.so.0";
#define LIBNAME glib2

static char* libname = NULL;

typedef void*(*pFp_t)(void*);
typedef void*(*pFpp_t)(void*, void*);
typedef void(*vFppip_t)(void*, void*, int, void*);
typedef void(*vFpppp_t)(void*, void*, void*, void*);

#define ADDED_FUNCTIONS()           \
GO(g_build_filenamev, pFp_t)        \
GO(g_variant_get_va, vFpppp_t)      \
GO(g_build_pathv, pFpp_t)           \
GO(g_set_error_literal, vFppip_t)   \

#include "generated/wrappedglib2types.h"

#include "wrappercallback.h"

EXPORT void* my_g_markup_vprintf_escaped(x86emu_t *emu, void* fmt, void* b) {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    return my->g_markup_vprintf_escaped(fmt, VARARGS);
    #else
    (void)emu;
    // other platform don't need that
    return my->g_markup_vprintf_escaped(fmt, b);
    #endif
}

EXPORT void* my_g_build_filename(x86emu_t* emu, void* first, void** b)
{
    (void)emu;
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
    return (int)RunFunctionFmt(sig->c_handler, "p", sig->data);
}
EXPORT uint32_t my_g_timeout_add(x86emu_t* emu, uint32_t interval, void* func, void* data)
{
    (void)emu;
    my_signal_t *sig = new_mysignal(func, data, NULL);
    return my->g_timeout_add_full(0, interval, my_timeout_cb, sig, my_signal_delete);
}
typedef int (*GSourceFunc) (void* user_data);

typedef struct my_GSourceFuncs_s {
  int  (*prepare)  (void* source, int* timeout_);
  int  (*check)    (void* source);
  int  (*dispatch) (void* source, GSourceFunc callback,void* user_data);
  void (*finalize) (void* source);
  GSourceFunc closure;
  void* marshal;
} my_GSourceFuncs_t;

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)   \
GO(5)   \
GO(6)   \
GO(7)   \
GO(8)   \
GO(9)   \

// GCopyFct
/*#define GO(A)   \
static uintptr_t my_copy_fct_##A = 0;   \
static void* my_copy_##A(void* data)     \
{                                       \
    return (void*)RunFunctionFmt(my_copy_fct_##A, "p", data);\
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
static uintptr_t my_free_fct_##A = 0;                       \
static void my_free_##A(void* data)                         \
{                                                           \
    RunFunctionFmt(my_free_fct_##A, "p", data); \
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
static uintptr_t my_duplicate_fct_##A = 0;                                      \
static void* my_duplicate_##A(void* data)                                       \
{                                                                               \
    return (void*)RunFunctionFmt(my_duplicate_fct_##A, "p", data);  \
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
static uintptr_t fct_funcs_prepare_##A = 0;                                                 \
static int my_funcs_prepare_##A(void* source, int* timeout_) {                              \
    return (int)RunFunctionFmt(fct_funcs_prepare_##A, "pp", source, timeout_);  \
}   \
static uintptr_t fct_funcs_check_##A = 0;                                       \
static int my_funcs_check_##A(void* source) {                                   \
    return (int)RunFunctionFmt(fct_funcs_check_##A, "p", source);   \
}   \
static uintptr_t fct_funcs_dispatch_cb_##A = 0;                                             \
static int my_funcs_dispatch_cb_##A(void* a, void* b, void* c, void* d) {                   \
    return (int)RunFunctionFmt(fct_funcs_dispatch_cb_##A, "pppp", a, b, c, d);  \
}   \
static uintptr_t fct_funcs_dispatch_##A = 0;                                                                                \
static int my_funcs_dispatch_##A(void* source, void* cb, void* data) {                                                      \
    uintptr_t old = fct_funcs_dispatch_cb_##A;                                                                              \
    fct_funcs_dispatch_cb_##A = (uintptr_t)cb;                                                                              \
    return (int)RunFunctionFmt(fct_funcs_dispatch_##A, "ppp", source, cb?my_funcs_dispatch_cb_##A:NULL, data);  \
    fct_funcs_dispatch_cb_##A = old;    \
}   \
static uintptr_t fct_funcs_finalize_##A = 0;                                    \
static int my_funcs_finalize_##A(void* source) {                                \
    return (int)RunFunctionFmt(fct_funcs_finalize_##A, "p", source);\
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
static uintptr_t my_poll_fct_##A = 0;                                               \
static int my_poll_##A(void* ufds, uint32_t nfsd, int32_t timeout_)                 \
{                                                                                   \
    return RunFunctionFmt(my_poll_fct_##A, "pui", ufds, nfsd, timeout_);\
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
    return (void*)AddCheckBridge(my_lib->w.bridge, iFpui, fct, 0, "GPollFunc");
}

// GHashFunc ...
#define GO(A)   \
static uintptr_t my_hashfunc_fct_##A = 0;                                       \
static uint32_t my_hashfunc_##A(void* key)                                      \
{                                                                               \
    return (uint32_t)RunFunctionFmt(my_hashfunc_fct_##A, "p", key); \
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
static uintptr_t my_equalfunc_fct_##A = 0;                              \
static int my_equalfunc_##A(void* a, void* b)                           \
{                                                                       \
    return RunFunctionFmt(my_equalfunc_fct_##A, "pp", a, b);\
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
static uintptr_t my_destroyfunc_fct_##A = 0;                                \
static int my_destroyfunc_##A(void* a, void* b)                             \
{                                                                           \
    return RunFunctionFmt(my_destroyfunc_fct_##A, "pp", a, b);  \
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
static uintptr_t my_spwnchildsetup_fct_##A = 0;                         \
static void my_spwnchildsetup_##A(void* data)                           \
{                                                                       \
    RunFunctionFmt(my_spwnchildsetup_fct_##A, "p", data);   \
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
static uintptr_t my_GSourceFunc_fct_##A = 0;                                \
static void my_GSourceFunc_##A(void* a, void* b, void* c, void* d)          \
{                                                                           \
    RunFunctionFmt(my_GSourceFunc_fct_##A, "pppp", a, b, c, d); \
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
static uintptr_t my_GCompareFunc_fct_##A = 0;                                   \
static int my_GCompareFunc_##A(void* a, void* b)                                \
{                                                                               \
    return (int)RunFunctionFmt(my_GCompareFunc_fct_##A, "pp", a, b);\
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
// GCompareDataFunc ...
#define GO(A)   \
static uintptr_t my_GCompareDataFunc_fct_##A = 0;                                           \
static int my_GCompareDataFunc_##A(void* a, void* b, void* data)                            \
{                                                                                           \
    return (int)RunFunctionFmt(my_GCompareDataFunc_fct_##A, "ppp", a, b, data); \
}
SUPER()
#undef GO
static void* findGCompareDataFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GCompareDataFunc_fct_##A == (uintptr_t)fct) return my_GCompareDataFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GCompareDataFunc_fct_##A == 0) {my_GCompareDataFunc_fct_##A = (uintptr_t)fct; return my_GCompareDataFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GCompareDataFunc callback\n");
    return NULL;
}
// GCompletionFunc ...
#define GO(A)   \
static uintptr_t my_GCompletionFunc_fct_##A = 0;                                    \
static void* my_GCompletionFunc_##A(void* a)                                        \
{                                                                                   \
    return (void*)RunFunctionFmt(my_GCompletionFunc_fct_##A, "p", a);   \
}
SUPER()
#undef GO
static void* findGCompletionFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GCompletionFunc_fct_##A == (uintptr_t)fct) return my_GCompletionFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GCompletionFunc_fct_##A == 0) {my_GCompletionFunc_fct_##A = (uintptr_t)fct; return my_GCompletionFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GCompletionFunc callback\n");
    return NULL;
}
// GCompletionStrncmpFunc ...
#define GO(A)   \
static uintptr_t my_GCompletionStrncmpFunc_fct_##A = 0;                                         \
static int my_GCompletionStrncmpFunc_##A(void* a, void* b, unsigned long n)                     \
{                                                                                               \
    return (int)RunFunctionFmt(my_GCompletionStrncmpFunc_fct_##A, "ppL", a, b, n);  \
}
SUPER()
#undef GO
static void* findGCompletionStrncmpFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GCompletionStrncmpFunc_fct_##A == (uintptr_t)fct) return my_GCompletionStrncmpFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GCompletionStrncmpFunc_fct_##A == 0) {my_GCompletionStrncmpFunc_fct_##A = (uintptr_t)fct; return my_GCompletionStrncmpFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GCompletionStrncmpFunc callback\n");
    return NULL;
}
// GIOFunc ...
#define GO(A)   \
static uintptr_t my_GIOFunc_fct_##A = 0;                                        \
static int my_GIOFunc_##A(void* a, int b, void* c)                              \
{                                                                               \
    return (int)RunFunctionFmt(my_GIOFunc_fct_##A, "pip", a, b, c); \
}
SUPER()
#undef GO
static void* findGIOFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GIOFunc_fct_##A == (uintptr_t)fct) return my_GIOFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GIOFunc_fct_##A == 0) {my_GIOFunc_fct_##A = (uintptr_t)fct; return my_GIOFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GIOFunc callback\n");
    return NULL;
}
// GDestroyNotify ...
#define GO(A)   \
static uintptr_t my_GDestroyNotify_fct_##A = 0;                     \
static void my_GDestroyNotify_##A(void* a)                          \
{                                                                   \
    RunFunctionFmt(my_GDestroyNotify_fct_##A, "p", a);  \
}
SUPER()
#undef GO
static void* findGDestroyNotifyFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GDestroyNotify_fct_##A == (uintptr_t)fct) return my_GDestroyNotify_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GDestroyNotify_fct_##A == 0) {my_GDestroyNotify_fct_##A = (uintptr_t)fct; return my_GDestroyNotify_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GDestroyNotify callback\n");
    return NULL;
}
// GFunc ...
#define GO(A)   \
static uintptr_t my_GFunc_fct_##A = 0;                          \
static void my_GFunc_##A(void* a, void* b)                      \
{                                                               \
    RunFunctionFmt(my_GFunc_fct_##A, "pp", a, b);   \
}
SUPER()
#undef GO
static void* findGFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GFunc_fct_##A == (uintptr_t)fct) return my_GFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GFunc_fct_##A == 0) {my_GFunc_fct_##A = (uintptr_t)fct; return my_GFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GFunc callback\n");
    return NULL;
}
// GHFunc ...
#define GO(A)   \
static uintptr_t my_GHFunc_fct_##A = 0;                             \
static void my_GHFunc_##A(void* a, void* b, void* c)                \
{                                                                   \
    RunFunctionFmt(my_GHFunc_fct_##A, "ppp", a, b, c);  \
}
SUPER()
#undef GO
static void* findGHFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GHFunc_fct_##A == (uintptr_t)fct) return my_GHFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GHFunc_fct_##A == 0) {my_GHFunc_fct_##A = (uintptr_t)fct; return my_GHFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GHFunc callback\n");
    return NULL;
}
// GHRFunc ...
#define GO(A)   \
static uintptr_t my_GHRFunc_fct_##A = 0;                                    \
static int my_GHRFunc_##A(void* a, void* b, void* c)                        \
{                                                                           \
    return RunFunctionFmt(my_GHRFunc_fct_##A, "ppp", a, b, c);  \
}
SUPER()
#undef GO
static void* findGHRFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GHRFunc_fct_##A == (uintptr_t)fct) return my_GHRFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GHRFunc_fct_##A == 0) {my_GHRFunc_fct_##A = (uintptr_t)fct; return my_GHRFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GHRFunc callback\n");
    return NULL;
}
// GChildWatchFunc ...
#define GO(A)   \
static uintptr_t my_GChildWatchFunc_fct_##A = 0;                            \
static void my_GChildWatchFunc_##A(int a, int b, void* c)                   \
{                                                                           \
    RunFunctionFmt(my_GChildWatchFunc_fct_##A, "iip", a, b, c); \
}
SUPER()
#undef GO
static void* findGChildWatchFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GChildWatchFunc_fct_##A == (uintptr_t)fct) return my_GChildWatchFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GChildWatchFunc_fct_##A == 0) {my_GChildWatchFunc_fct_##A = (uintptr_t)fct; return my_GChildWatchFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GChildWatchFunc callback\n");
    return NULL;
}
// GLogFunc ...
#define GO(A)   \
static uintptr_t my_GLogFunc_fct_##A = 0;                               \
static void my_GLogFunc_##A(void* a, int b, void* c, void* d)           \
{                                                                       \
    RunFunctionFmt(my_GLogFunc_fct_##A, "pipp", a, b, c, d);\
}
SUPER()
#undef GO
static void* findGLogFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GLogFunc_fct_##A == (uintptr_t)fct) return my_GLogFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GLogFunc_fct_##A == 0) {my_GLogFunc_fct_##A = (uintptr_t)fct; return my_GLogFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GLogFunc callback\n");
    return NULL;
}
static void* reverseGLogFuncFct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if((uintptr_t)fct == my_GLogFunc_fct_##A) return (void*)my_GLogFunc_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddCheckBridge(my_lib->w.bridge, iFpui, fct, 0, "GLogFunc");
}
// GPrintFunc ...
#define GO(A)   \
static uintptr_t my_GPrintFunc_fct_##A = 0;                     \
static void my_GPrintFunc_##A(void* a)                          \
{                                                               \
    RunFunctionFmt(my_GPrintFunc_fct_##A, "p", a);  \
}
SUPER()
#undef GO
static void* findGPrintFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GPrintFunc_fct_##A == (uintptr_t)fct) return my_GPrintFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GPrintFunc_fct_##A == 0) {my_GPrintFunc_fct_##A = (uintptr_t)fct; return my_GPrintFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GPrintFunc callback\n");
    return NULL;
}
static void* reverseGPrintFuncFct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if((uintptr_t)fct == my_GPrintFunc_fct_##A) return (void*)my_GPrintFunc_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddCheckBridge(my_lib->w.bridge, vFp, fct, 0, "GPrintFunc");
}

// GOptionArg ...
#define GO(A)   \
static uintptr_t my_GOptionArg_fct_##A = 0;                                             \
static int my_GOptionArg_##A(void* a, void* b, void* c, void* d)                        \
{                                                                                       \
    return (int)RunFunctionFmt(my_GOptionArg_fct_##A, "pppp", a, b, c, d);  \
}
SUPER()
#undef GO
static void* findGOptionArgFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GOptionArg_fct_##A == (uintptr_t)fct) return my_GOptionArg_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GOptionArg_fct_##A == 0) {my_GOptionArg_fct_##A = (uintptr_t)fct; return my_GOptionArg_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GOptionArg callback\n");
    return NULL;
}
static void* reverseGOptionArgFct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if((uintptr_t)fct == my_GOptionArg_fct_##A) return (void*)my_GOptionArg_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddCheckBridge(my_lib->w.bridge, iFpppp, fct, 0, "GOptionFunc");
}
#undef SUPER

EXPORT void my_g_list_free_full(x86emu_t* emu, void* list, void* free_func)
{
    (void)emu;
    my->g_list_free_full(list, findFreeFct(free_func));
}

EXPORT void* my_g_markup_printf_escaped(x86emu_t *emu, void* fmt, void* b) {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    return my->g_markup_vprintf_escaped(fmt, VARARGS);
    #else
    (void)emu;
    // other platform don't need that
    return my->g_markup_vprintf_escaped(fmt, b);
    #endif
}


EXPORT void my_g_datalist_id_set_data_full(x86emu_t* emu, void* datalist, uint32_t key, void* data, void* freecb)
{
    (void)emu;
    void* fc = findFreeFct(freecb);
    my->g_datalist_id_set_data_full(datalist, key, data, fc);
}

EXPORT void* my_g_datalist_id_dup_data(x86emu_t* emu, void* datalist, uint32_t key, void* dupcb, void* data)
{
    (void)emu;
    void* cc = findDuplicateFct(dupcb);
    return my->g_datalist_id_dup_data(datalist, key, cc, data);
}

EXPORT int my_g_datalist_id_replace_data(x86emu_t* emu, void* datalist, uint32_t key, void* oldval, void* newval, void* oldfree, void* newfree)
{
    (void)emu;
    void* oldfc = findFreeFct(oldfree);
    void* newfc = findFreeFct(newfree);
    return my->g_datalist_id_replace_data(datalist, key, oldval, newval, oldfc, newfc);
}

EXPORT void* my_g_variant_new_from_data(x86emu_t* emu, void* type, void* data, uint32_t size, int trusted, void* freecb, void* datacb)
{
    (void)emu;
    void* fc = findFreeFct(freecb);
    return my->g_variant_new_from_data(type, data, size, trusted, fc, datacb);
}

EXPORT void* my_g_variant_new_parsed_va(x86emu_t* emu, void* fmt, void* b)
{
    #ifndef NOALIGN
    myStackAlignGVariantNew((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    uint32_t *aligned = VARARGS;
    return my->g_variant_new_parsed_va(fmt, &aligned);
    #else
    (void)emu;
    return my->g_variant_new_parsed_va(fmt, b);
    #endif
}

EXPORT void my_g_variant_get(x86emu_t* emu, void* value, void* fmt, uint32_t* b)
{
    (void)emu;
    my->g_variant_get_va(value, fmt, NULL, &b);
}

EXPORT void* my_g_strdup_vprintf(x86emu_t* emu, void* fmt, void* b)
{
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    return my->g_strdup_vprintf(fmt, VARARGS);
    #else
    (void)emu;
    return my->g_strdup_vprintf(fmt, b);
    #endif
}

EXPORT int my_g_vprintf(x86emu_t* emu, void* fmt, void* b)
{
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    return my->g_vprintf(fmt, VARARGS);
    #else
    (void)emu;
    return my->g_vprintf(fmt, b);
    #endif
}

EXPORT int my_g_vfprintf(x86emu_t* emu, void* F, void* fmt, void* b)
{
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    return my->g_vfprintf(F, fmt, VARARGS);
    #else
    (void)emu;
    return my->g_vfprintf(F, fmt, b);
    #endif
}

EXPORT int my_g_vsprintf(x86emu_t* emu, void* s, void* fmt, void* b)
{
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    return my->g_vsprintf(s, fmt, VARARGS);
    #else
    (void)emu;
    return my->g_vsprintf(s, fmt, b);
    #endif
}

EXPORT int my_g_vsnprintf(x86emu_t* emu, void* s, unsigned long n, void* fmt, void* b)
{
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    return my->g_vsnprintf(s, n, fmt, VARARGS);
    #else
    (void)emu;
    return my->g_vsnprintf(s, n, fmt, b);
    #endif
}

EXPORT int my_g_vasprintf(x86emu_t* emu, void* s, void* fmt, void* b)
{
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    return my->g_vasprintf(s, fmt, VARARGS);
    #else
    (void)emu;
    return my->g_vasprintf(s, fmt, b);
    #endif
}

EXPORT uint32_t my_g_printf_string_upper_bound(x86emu_t* emu, void* fmt, void* b)
{
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    return my->g_printf_string_upper_bound(fmt, VARARGS);
    #else
    (void)emu;
    return my->g_printf_string_upper_bound(fmt, b);
    #endif
}

EXPORT void my_g_print(x86emu_t* emu, void* fmt, void* b)
{

    char* buf = NULL;
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    iFppp_t f = (iFppp_t)vasprintf;
    f(&buf, fmt, VARARGS);
    #else
    (void)emu;
    iFppp_t f = (iFppp_t)vasprintf;
    f(&buf, fmt, b);
    #endif
    my->g_print(buf);
    free(buf);
}

EXPORT void my_g_printerr(x86emu_t* emu, void* fmt, void* b)
{

    char* buf = NULL;
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    iFppp_t f = (iFppp_t)vasprintf;
    f(&buf, fmt, VARARGS);
    #else
    (void)emu;
    iFppp_t f = (iFppp_t)vasprintf;
    f(&buf, fmt, b);
    #endif
    my->g_printerr(buf);
    free(buf);
}

EXPORT void* my_g_source_new(x86emu_t* emu, my_GSourceFuncs_t* source_funcs, uint32_t struct_size)
{
    (void)emu;
    return my->g_source_new(findFreeGSourceFuncs(source_funcs), struct_size);
}

EXPORT void my_g_source_set_funcs(x86emu_t* emu, void* source, my_GSourceFuncs_t* source_funcs)
{
    (void)emu;
    my->g_source_set_funcs(source, findFreeGSourceFuncs(source_funcs));
}


EXPORT int my_g_source_remove_by_funcs_user_data(x86emu_t* emu, my_GSourceFuncs_t* source_funcs, void* data)
{
    (void)emu;
    return my->g_source_remove_by_funcs_user_data(findFreeGSourceFuncs(source_funcs), data);
}

EXPORT void* my_g_main_context_get_poll_func(x86emu_t* emu, void* context)
{
    (void)emu;
    void* ret = my->g_main_context_get_poll_func(context);
    if(!ret) return ret;
    void* r = reversePollFct(ret);
    if(r) return r;
    // needs to bridge....
    return (void*)AddCheckBridge(my_lib->w.bridge, iFpui, ret, 0, NULL);
}
    
EXPORT void my_g_main_context_set_poll_func(x86emu_t* emu, void* context, void* func)
{
    (void)emu;
    my->g_main_context_set_poll_func(context, findPollFct(func));
}

EXPORT uint32_t my_g_idle_add_full(x86emu_t* emu, int priority, void* f, void* data, void* notify)
{
    (void)emu;
    if(!f)
        return my->g_idle_add_full(priority, f, data, notify);

    my_signal_t *sig = new_mysignal(f, data, notify);
    printf_log(LOG_DEBUG, "glib2 Idle CB with priority %d created for %p, sig=%p\n", priority, f, sig);
    return my->g_idle_add_full(priority, my_timeout_cb, sig, my_signal_delete);
}

EXPORT void* my_g_hash_table_new(x86emu_t* emu, void* hash, void* equal)
{
    (void)emu;
    return my->g_hash_table_new(findHashFct(hash), findEqualFct(equal));
}

EXPORT void* my_g_hash_table_new_full(x86emu_t* emu, void* hash, void* equal, void* destroy_key, void* destroy_val)
{
    (void)emu;
    return my->g_hash_table_new_full(findHashFct(hash), findEqualFct(equal), findDestroyFct(destroy_key), findDestroyFct(destroy_val));
}

EXPORT void my_g_hash_table_foreach(x86emu_t* emu, void* table, void* f, void* data)
{
    (void)emu;
    my->g_hash_table_foreach(table, findGHFuncFct(f), data);
}

EXPORT uint32_t my_g_hash_table_foreach_remove(x86emu_t* emu, void* table, void* f, void* data)
{
    (void)emu;
    return my->g_hash_table_foreach_remove(table, findGHRFuncFct(f), data);
}
EXPORT uint32_t my_g_hash_table_foreach_steal(x86emu_t* emu, void* table, void* f, void* data)
{
    (void)emu;
    return my->g_hash_table_foreach_steal(table, findGHRFuncFct(f), data);
}
EXPORT void* my_g_hash_table_find(x86emu_t* emu, void* table, void* f, void* data)
{
    (void)emu;
    return my->g_hash_table_find(table, findGHRFuncFct(f), data);
}

EXPORT int my_g_spawn_async_with_pipes(x86emu_t* emu, void* dir, void* argv, void* envp, int flags, void* f, void* data, void* child, void* input, void* output, void* err, void* error)
{
    (void)emu;
    return my->g_spawn_async_with_pipes(dir, argv, envp, flags, findSpawnChildSetupFct(f), data, child, input, output, err, error);
}

EXPORT int my_g_spawn_async(x86emu_t* emu, void* dir, void* argv, void* envp, int flags, void* f, void* data, void* child, void* error)
{
    (void)emu;
    return my->g_spawn_async(dir, argv, envp, flags, findSpawnChildSetupFct(f), data, child, error);
}

EXPORT int my_g_spawn_sync(x86emu_t* emu, void* dir, void* argv, void* envp, int flags, void* f, void* data, void* input, void* output, void* status, void* error)
{
    (void)emu;
    return my->g_spawn_sync(dir, argv, envp, flags, findSpawnChildSetupFct(f), data, input, output, status, error);
}

EXPORT uint32_t my_g_child_watch_add(x86emu_t* emu, int pid, void* f, void* data)
{
    (void)emu;
    return my->g_child_watch_add(pid, findGChildWatchFuncFct(f), data);
}

EXPORT uint32_t my_g_child_watch_add_full(x86emu_t* emu, int priority, int pid, void* f, void* data, void* notify)
{
    (void)emu;
    return my->g_child_watch_add_full(priority, pid, findGChildWatchFuncFct(f), data, findGDestroyNotifyFct(notify));
}

EXPORT void* my_g_private_new(x86emu_t* emu, void* notify)
{
    (void)emu;
    return my->g_private_new(findFreeFct(notify));
}

EXPORT void my_g_static_private_set(x86emu_t* emu, void* private, void* data, void* notify)
{
    (void)emu;
    my->g_static_private_set(private, data, findFreeFct(notify));
}

EXPORT void* my_g_ptr_array_new_with_free_func(x86emu_t* emu, void* notify)
{
    (void)emu;
    return my->g_ptr_array_new_with_free_func(findFreeFct(notify));
}

EXPORT void* my_g_ptr_array_new_full(x86emu_t* emu, uint32_t size, void* notify)
{
    (void)emu;
    return my->g_ptr_array_new_full(size, findFreeFct(notify));
}

EXPORT void my_g_ptr_array_set_free_func(x86emu_t* emu, void* array, void* notify)
{
    (void)emu;
    my->g_ptr_array_set_free_func(array, findFreeFct(notify));
}

EXPORT void my_g_ptr_array_sort(x86emu_t* emu, void* array, void* comp)
{
    (void)emu;
    my->g_ptr_array_sort(array, findGCompareFuncFct(comp));
}

EXPORT void my_g_ptr_array_sort_with_data(x86emu_t* emu, void* array, void* comp, void* data)
{
    (void)emu;
    my->g_ptr_array_sort_with_data(array, findGCompareDataFuncFct(comp), data);
}

EXPORT void my_g_qsort_with_data(x86emu_t* emu, void* pbase, int total, unsigned long size, void* comp, void* data)
{
    (void)emu;
    my->g_qsort_with_data(pbase, total, size, findGCompareDataFuncFct(comp), data);
}

EXPORT void my_g_ptr_array_foreach(x86emu_t* emu, void* array, void* func, void* data)
{
    (void)emu;
    my->g_ptr_array_foreach(array, findGFuncFct(func), data);
}

EXPORT void* my_g_thread_create(x86emu_t* emu, void* func, void* data, int joinable, void* error)
{

    void* et = NULL;
    return my->g_thread_create(my_prepare_thread(emu, func, data, 0, &et), et, joinable, error);
}

EXPORT void* my_g_thread_create_full(x86emu_t* emu, void* func, void* data, unsigned long stack, int joinable, int bound, int priority, void* error)
{

    void* et = NULL;
    return my->g_thread_create_full(my_prepare_thread(emu, func, data, stack, &et), et, stack, joinable, bound, priority, error);
}

EXPORT void my_g_thread_foreach(x86emu_t* emu, void* func, void* data)
{
    (void)emu;
    my->g_thread_foreach(findGFuncFct(func), data);
}

EXPORT void my_g_array_sort(x86emu_t* emu, void* array, void* comp)
{
    (void)emu;
    my->g_array_sort(array, findGCompareFuncFct(comp));
}

EXPORT void my_g_array_sort_with_data(x86emu_t* emu, void* array, void* comp, void* data)
{
    (void)emu;
    my->g_array_sort_with_data(array, findGCompareDataFuncFct(comp), data);
}

EXPORT void my_g_array_set_clear_func(x86emu_t* emu, void* array, void* notify)
{
    (void)emu;
    my->g_array_set_clear_func(array, findFreeFct(notify));
}

EXPORT void my_g_source_set_callback(x86emu_t* emu, void* source, void* func, void* data, void* notify)
{
    (void)emu;
    my->g_source_set_callback(source, findGSourceFuncFct(func), data, findFreeFct(notify));
}

EXPORT void* my_g_slist_insert_sorted(x86emu_t* emu, void* list, void* d, void* comp)
{
    (void)emu;
    return my->g_slist_insert_sorted(list, d, findGCompareFuncFct(comp));
}
EXPORT void* my_g_slist_insert_sorted_with_data(x86emu_t* emu, void* list, void* d, void* comp, void* data)
{
    (void)emu;
    return my->g_slist_insert_sorted_with_data(list, d, findGCompareDataFuncFct(comp), data);
}

EXPORT void my_g_slist_foreach(x86emu_t* emu, void* list, void* func, void* data)
{
    (void)emu;
    my->g_slist_foreach(list, findGFuncFct(func), data);
}

EXPORT void* my_g_slist_find_custom(x86emu_t* emu, void* list, void* data, void* comp)
{
    (void)emu;
    return my->g_slist_find_custom(list, data, findGCompareFuncFct(comp));
}

EXPORT uint32_t my_g_idle_add(x86emu_t* emu, void* func, void* data)
{
    (void)emu;
    return my->g_idle_add(findGSourceFuncFct(func), data);
}

EXPORT void* my_g_variant_new_va(x86emu_t* emu, char* fmt, void* endptr, uint32_t** b)
{
    myStackAlignGVariantNew(fmt, *b, emu->scratch);
    PREPARE_VALIST;
    uint32_t* aligned = VARARGS;
    return my->g_variant_new_va(fmt, endptr, &aligned);
}

EXPORT void* my_g_variant_new(x86emu_t* emu, char* fmt, uint32_t* b)
{
    return my_g_variant_new_va(emu, fmt, NULL, &b);
}

EXPORT void* my_g_completion_new(x86emu_t* emu, void* f)
{
    (void)emu;
    return my->g_completion_new(findGCompletionFct(f));
}

EXPORT void my_g_completion_set_compare(x86emu_t *emu, void* cmp, void* f)
{
    (void)emu;
    my->g_completion_set_compare(cmp, findGCompletionStrncmpFuncFct(f));
}

EXPORT void* my_g_log_set_default_handler(x86emu_t *emu, void* f, void* data)
{
    (void)emu;
    return reverseGLogFuncFct(my->g_log_set_default_handler(findGLogFuncFct(f), data));
}

EXPORT uint32_t my_g_io_add_watch_full(x86emu_t* emu, void* channel, int priority, int cond, void* f, void* data, void* notify)
{
    (void)emu;
    return my->g_io_add_watch_full(channel, priority, cond, findGIOFuncFct(f), data, findDestroyFct(notify));
}

EXPORT uint32_t my_g_io_add_watch(x86emu_t* emu, void* channel, int cond, void* f, void* data)
{
    (void)emu;
    return my->g_io_add_watch(channel, cond, findGIOFuncFct(f), data);
}

EXPORT void* my_g_set_print_handler(x86emu_t *emu, void* f)
{
    (void)emu;
    return reverseGPrintFuncFct(my->g_set_print_handler(findGPrintFuncFct(f)));
}

EXPORT void* my_g_set_printerr_handler(x86emu_t *emu, void* f)
{
    (void)emu;
    return reverseGPrintFuncFct(my->g_set_printerr_handler(findGPrintFuncFct(f)));
}

EXPORT void* my_g_slist_sort(x86emu_t *emu, void* list, void* f)
{
    (void)emu;
    return my->g_slist_sort(list, findGCompareFuncFct(f));
}

EXPORT void* my_g_slist_sort_with_data(x86emu_t *emu, void* list, void* f, void* data)
{
    (void)emu;
    return my->g_slist_sort_with_data(list, findGCompareDataFuncFct(f), data);
}

EXPORT void* my_g_build_path(x86emu_t *emu, void* sep, void* first, void** data)
{
    (void)emu;
    int n = (first)?1:0;
    void* p = n?data[0]:NULL;
    while(p) {
        p = data[n++];
    }
    ++n;    // final NULL
    void** args = (void**)alloca(n *sizeof(void*));
    args[0] = first;
    for(int i=1; i<n; ++i)
        args[i] = data[i-1];
    p = my->g_build_pathv(sep, args);
    return p;
}

EXPORT void* my_g_list_sort(x86emu_t *emu, void* list, void* f)
{
    (void)emu;
    return my->g_list_sort(list, findGCompareFuncFct(f));
}

EXPORT void* my_g_list_sort_with_data(x86emu_t *emu, void* list, void* f, void* data)
{
    (void)emu;
    return my->g_list_sort_with_data(list, findGCompareDataFuncFct(f), data);
}

EXPORT void* my_g_queue_find_custom(x86emu_t *emu, void* queue, void* data, void* f)
{
    (void)emu;
    return my->g_queue_find_custom(queue, data, findGCompareFuncFct(f));
}

EXPORT void* my_g_list_find_custom(x86emu_t *emu, void* list, void* data, void* f)
{
    (void)emu;
    return my->g_list_find_custom(list, data, findGCompareFuncFct(f));
}

EXPORT uint32_t my_g_timeout_add_full(x86emu_t *emu, int priority, uint32_t interval, void* f, void* data, void* notify)
{
    (void)emu;
    return my->g_timeout_add_full(priority, interval, findGSourceFuncFct(f), data, findDestroyFct(notify));
}

EXPORT uint32_t my_g_timeout_add_seconds(x86emu_t *emu, uint32_t interval, void* f, void* data)
{
    (void)emu;
    return my->g_timeout_add_seconds(interval, findGSourceFuncFct(f), data);
}

EXPORT uint32_t my_g_timeout_add_seconds_full(x86emu_t *emu, int priority, uint32_t interval, void* f, void* data, void* notify)
{
    (void)emu;
    return my->g_timeout_add_seconds_full(priority, interval, findGSourceFuncFct(f), data, findDestroyFct(notify));
}

EXPORT uint32_t my_g_log_set_handler(x86emu_t *emu, void* domain, int level, void* f, void* data)
{
    (void)emu;
    return my->g_log_set_handler(domain, level, findGLogFuncFct(f), data);
}

EXPORT void my_g_set_error(x86emu_t *emu, void* err, void* domain, int code, void* fmt, uint32_t* stack)
{
    char buf[1000];
    myStackAlign(fmt, stack, emu->scratch);
    void* f = vsnprintf;
    ((iFpLpp_t)f)(buf, sizeof(buf), fmt, emu->scratch);
    my->g_set_error_literal(err, domain, code, buf);
}

typedef struct my_GOptionEntry_s {
  void*     long_name;
  char      short_name;
  int       flags;
  int       arg;
  void*     arg_data;
  void*     description;
  void*     arg_description;
} my_GOptionEntry_t;

EXPORT void my_g_option_context_add_main_entries(x86emu_t* emu, void* context, my_GOptionEntry_t entries[], void* domain)
{
    (void)emu;
    my_GOptionEntry_t* p = entries;
    while (p->long_name) {
        // wrap Callbacks
        if (p->arg == 3)
            p->arg_data = findGOptionArgFct(p->arg_data);
        ++p;
    }
    my->g_option_context_add_main_entries(context, entries, domain);
    p = entries;
    while (p->long_name) {
        // unwrap Callbacks
        if (p->arg == 3)
            p->arg_data = reverseGOptionArgFct(p->arg_data);
        ++p;
    }
}

EXPORT void my_g_list_foreach(x86emu_t* emu, void* list, void* f, void* data)
{
    (void)emu;
    my->g_list_foreach(list, findGFuncFct(f), data);
}

#define PRE_INIT    \
    if(box86_nogtk) \
        return -1;

#define CUSTOM_INIT \
    libname = lib->name;\
    my_lib = lib;       \
    getMy(lib);

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"

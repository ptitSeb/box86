#include <stdio.h>
#include <string.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <syscall.h>
#include <stddef.h>
#include <stdarg.h>
#include <fts.h>

#include "box86context.h"
#include "debug.h"
#include "x86emu.h"
#include "emu/x86emu_private.h"
#include "signals.h"
#include "box86stack.h"
#include "dynarec.h"
#include "callback.h"
#include "myalign.h"
#include "bridge.h"
#include "custommem.h"
#include "khash.h"

// kh_ftsent_t store each ftsent conversion from native -> x86
KHASH_MAP_INIT_INT(ftsent, x86_ftsent_t*)

// kh_fts_t store a kh_ftsent_t for each fts...
KHASH_MAP_INIT_INT(fts, kh_ftsent_t*)

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)

// compare
#ifdef NOALIGN
#define GO(A)   \
static uintptr_t my_compare_fct_##A = 0;                                    \
static int my_compare_##A(FTSENT* a, FTSENT* b)                             \
{                                                                           \
    return (int)RunFunctionFmt(my_compare_fct_##A, "pp", a, b); \
}
#else
#define GO(A)   \
static uintptr_t my_compare_fct_##A = 0;   \
static int my_compare_##A(FTSENT* a, FTSENT* b)                                     \
{                                                                                   \
    x86_ftsent_t x86_a, x86_b;                                                      \
    UnalignFTSENT(&x86_a, a);                                                       \
    UnalignFTSENT(&x86_b, b);                                                       \
    return (int)RunFunctionFmt(my_compare_fct_##A, "pp", x86_a, x86_b); \
}
#endif
SUPER()
#undef GO
static void* findcompareFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_compare_fct_##A == (uintptr_t)fct) return my_compare_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_compare_fct_##A == 0) {my_compare_fct_##A = (uintptr_t)fct; return my_compare_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libpng12 compare callback\n");
    return NULL;
}
#undef SUPER

#ifndef NOALIGN
kh_ftsent_t* getFtsentMap(box86context_t* context, void* ftsp)
{
    khint_t k;
    int ret;

    kh_fts_t *ftsmap = context->ftsmap;
    
    k = kh_get(fts, ftsmap, (uintptr_t)ftsp);
    if(k==kh_end(ftsmap)) {
        k = kh_put(fts, ftsmap, (uintptr_t)ftsp, &ret);
        kh_value(ftsmap, k) = kh_init(ftsent);
    }
    return kh_value(ftsmap, k);
}

int existFtsent(box86context_t* context, void* ftsp, FTSENT* ftsent)
{
    khint_t k;
    kh_ftsent_t *ftsentMap = getFtsentMap(context, ftsp);

    k = kh_get(ftsent, ftsentMap, (uintptr_t)ftsent);
    return (k==kh_end(ftsentMap))?0:1;
}

x86_ftsent_t* getFtsent(kh_ftsent_t* ftsentMap, FTSENT* ftsent, int dolink)
{
    if(!ftsent)
        return NULL;
    khint_t k;
    int ret;

    k = kh_get(ftsent, ftsentMap, (uintptr_t)ftsent);
    if(k!=kh_end(ftsentMap))
        return kh_value(ftsentMap, k);
    // new value... so need convertion & alignment
    k = kh_put(ftsent, ftsentMap, (uintptr_t)ftsent, &ret);
    x86_ftsent_t *x86ftsent = kh_value(ftsentMap, k) = (x86_ftsent_t*)box_malloc(sizeof(x86_ftsent_t));
    UnalignFTSENT(x86ftsent, ftsent);   // unalign
    // handle the 3 embedded ftsent...
    if(ftsent->fts_info<=14 && ftsent->fts_info!=7) {   //14 is max value, 7 is err
        if(ftsent->fts_info == FTS_DC)
            x86ftsent->fts_cycle = getFtsent(ftsentMap, ftsent->fts_cycle, dolink);
        if(ftsent->fts_level>=0) {
            x86ftsent->fts_parent = getFtsent(ftsentMap, ftsent->fts_parent, dolink);
            if(dolink)
                x86ftsent->fts_link = getFtsent(ftsentMap, ftsent->fts_link, dolink);
        }
    }
    return x86ftsent;
}

void freeFts(box86context_t* context, void* ftsp)
{
    khint_t k;

    kh_fts_t *ftsmap = context->ftsmap;
    
    k = kh_get(fts, ftsmap, (uintptr_t)ftsp);
    if(k==kh_end(ftsmap))
        return; // nothing to do...

    kh_ftsent_t *ftsentMap = kh_value(ftsmap, k);
    x86_ftsent_t* x86ftsent;
    kh_foreach_value(ftsentMap, x86ftsent, box_free(x86ftsent));
    kh_destroy(ftsent, ftsentMap);
    kh_del(fts, ftsmap, k);
}

#endif
EXPORT void* my_fts_open(x86emu_t* emu, void* path, int options, void* compare_fn)
{
    (void)emu;
    return fts_open(path, options, findcompareFct(compare_fn));
}

EXPORT int my_fts_close(x86emu_t* emu, void* ftsp)
{
    #ifndef NOALIGN
    freeFts(emu->context, ftsp);
    #else
    (void)emu;
    #endif
    return fts_close(ftsp);
}

EXPORT void* my_fts_read(x86emu_t* emu, void* ftsp)
{
    #ifdef NOALIGN
    (void)emu;
    return fts_read(ftsp);
    #else
    return getFtsent(getFtsentMap(emu->context, ftsp), fts_read(ftsp), 0);
    #endif
}

EXPORT void* my_fts_children(x86emu_t* emu, void* ftsp, int options)
{
    #ifdef NOALIGN
    (void)emu;
    return fts_children(ftsp, options);
    #else
    return getFtsent(getFtsentMap(emu->context, ftsp), fts_children(ftsp, options), 1);
    #endif
}

void InitFTSMap(box86context_t* context)
{
#ifndef NOALIGN
    context->ftsmap = kh_init(fts);
#else
    (void)context;
#endif
}
void FreeFTSMap(box86context_t* context)
{
#ifndef NOALIGN
    uintptr_t ptr;
    kh_ftsent_t *ftsentMap;
    (void)ftsentMap;
    kh_foreach(context->ftsmap, ptr, ftsentMap, freeFts(context, (FTSENT*)ptr));
    kh_destroy(fts, context->ftsmap);
#else
    (void)context;
#endif
}

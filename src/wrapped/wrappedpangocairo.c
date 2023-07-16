#define _GNU_SOURCE         /* See feature_test_macros(7) */
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

const char* pangocairoName = "libpangocairo-1.0.so.0";
#define LIBNAME pangocairo

#include "generated/wrappedpangocairotypes.h"

#define ADDED_FUNCTIONS()           \

#include "wrappercallback.h"

#undef SUPER

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// ShapeRenderer ...
#define GO(A)   \
static uintptr_t my_ShapeRenderer_fct_##A = 0;                                  \
static void my_ShapeRenderer_##A(void* a, void* b, int c, void* d)              \
{                                                                               \
    RunFunctionFmt(my_ShapeRenderer_fct_##A, "ppip", a, b, c, d);   \
}
SUPER()
#undef GO
static void* find_ShapeRenderer_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_ShapeRenderer_fct_##A == (uintptr_t)fct) return my_ShapeRenderer_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_ShapeRenderer_fct_##A == 0) {my_ShapeRenderer_fct_##A = (uintptr_t)fct; return my_ShapeRenderer_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for PangoCairo ShapeRenderer callback\n");
    return NULL;
}
static void* reverse_ShapeRenderer_Fct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_ShapeRenderer_##A == fct) return (void*)my_ShapeRenderer_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, vFppip, fct, 0, NULL);
}
// GDestroyNotify
#define GO(A)   \
static uintptr_t my_GDestroyNotify_fct_##A = 0;                         \
static void my_GDestroyNotify_##A(void* data)                           \
{                                                                       \
    RunFunctionFmt(my_GDestroyNotify_fct_##A, "p", data);   \
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
    printf_log(LOG_NONE, "Warning, no more slot for PangoCairo GDestroyNotify callback\n");
    return NULL;
}

#undef SUPER

EXPORT void* my_pango_cairo_context_get_shape_renderer(x86emu_t* emu, void* ctx, void* p)
{
    (void)emu;
    return reverse_ShapeRenderer_Fct(my->pango_cairo_context_get_shape_renderer(ctx, p));
}

EXPORT void my_pango_cairo_context_set_shape_renderer(x86emu_t* emu, void* ctx, void* f, void* p, void* d)
{
    (void)emu;
    my->pango_cairo_context_set_shape_renderer(ctx, find_ShapeRenderer_Fct(f), p, findGDestroyNotifyFct(d));
}

#define PRE_INIT    \
    if(box86_nogtk) \
        return -1;

#define CUSTOM_INIT \
    getMy(lib); \
    setNeededLibs(lib, 1, "libpango-1.0.so.0");

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"

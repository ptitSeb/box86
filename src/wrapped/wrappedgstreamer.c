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
#include "gtkclass.h"

#ifdef ANDROID
    const char* gstreamerName = "libgstreamer-1.0.so";
#else
    const char* gstreamerName = "libgstreamer-1.0.so.0";
#endif

#define LIBNAME gstreamer

typedef size_t  (*LFv_t)();
typedef void*   (*pFv_t)();
typedef void*   (*pFp_t)(void*);
typedef void    (*vFpp_t)(void*, void*);
typedef int     (*iFpp_t)(void*, void*);
typedef void*   (*pFppp_t)(void*, void*, void*);

#define ADDED_FUNCTIONS()                   \
    GO(gst_object_get_type, LFv_t)          \
    GO(gst_allocator_get_type, LFv_t)       \
    GO(gst_structure_new_empty, pFp_t)      \
    GO(gst_structure_new_valist, pFppp_t)   \
    GO(gst_caps_new_empty, pFv_t)           \
    GO(gst_caps_replace, iFpp_t)            \
    GO(gst_caps_append_structure, vFpp_t)   \

#include "generated/wrappedgstreamertypes.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \

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
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_destroyfunc_fct_##A == (uintptr_t)fct) return my_destroyfunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_destroyfunc_fct_##A == 0) {my_destroyfunc_fct_##A = (uintptr_t)fct; return my_destroyfunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GDestroyNotify callback\n");
    return NULL;
}
//GstPadActivateModeFunction
#define GO(A)   \
static uintptr_t my_GstPadActivateModeFunction_fct_##A = 0;                                             \
static int my_GstPadActivateModeFunction_##A(void* a, void* b, int c, int d)                            \
{                                                                                                       \
    return (int)RunFunctionFmt(my_GstPadActivateModeFunction_fct_##A, "ppii", a, b, c, d);  \
}
SUPER()
#undef GO
static void* findGstPadActivateModeFunctionFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_GstPadActivateModeFunction_fct_##A == (uintptr_t)fct) return my_GstPadActivateModeFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstPadActivateModeFunction_fct_##A == 0) {my_GstPadActivateModeFunction_fct_##A = (uintptr_t)fct; return my_GstPadActivateModeFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstPadActivateModeFunction callback\n");
    return NULL;
}
//GstPadQueryFunction
#define GO(A)   \
static uintptr_t my_GstPadQueryFunction_fct_##A = 0;                                        \
static int my_GstPadQueryFunction_##A(void* a, void* b, void* c)                            \
{                                                                                           \
    return (int)RunFunctionFmt(my_GstPadQueryFunction_fct_##A, "ppp", a, b, c); \
}
SUPER()
#undef GO
static void* findGstPadQueryFunctionFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_GstPadQueryFunction_fct_##A == (uintptr_t)fct) return my_GstPadQueryFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstPadQueryFunction_fct_##A == 0) {my_GstPadQueryFunction_fct_##A = (uintptr_t)fct; return my_GstPadQueryFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstPadQueryFunction callback\n");
    return NULL;
}
//GstPadGetRangeFunction
#define GO(A)   \
static uintptr_t my_GstPadGetRangeFunction_fct_##A = 0;                                                 \
static int my_GstPadGetRangeFunction_##A(void* a, void* b, uint64_t c, uint32_t d, void* e)             \
{                                                                                                       \
    return (int)RunFunctionFmt(my_GstPadGetRangeFunction_fct_##A, "ppUup", a, b, c, d, e);  \
}
SUPER()
#undef GO
static void* findGstPadGetRangeFunctionFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_GstPadGetRangeFunction_fct_##A == (uintptr_t)fct) return my_GstPadGetRangeFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstPadGetRangeFunction_fct_##A == 0) {my_GstPadGetRangeFunction_fct_##A = (uintptr_t)fct; return my_GstPadGetRangeFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstPadGetRangeFunction callback\n");
    return NULL;
}
//GstPadChainFunction
#define GO(A)   \
static uintptr_t my_GstPadChainFunction_fct_##A = 0;                                        \
static int my_GstPadChainFunction_##A(void* a, void* b, void* c)                            \
{                                                                                           \
    return (int)RunFunctionFmt(my_GstPadChainFunction_fct_##A, "ppp", a, b, c); \
}
SUPER()
#undef GO
static void* findGstPadChainFunctionFct(void* fct)
{
    if(!fct) return fct;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_GstPadChainFunction_fct_##A == (uintptr_t)fct) return my_GstPadChainFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstPadChainFunction_fct_##A == 0) {my_GstPadChainFunction_fct_##A = (uintptr_t)fct; return my_GstPadChainFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstPadChainFunction callback\n");
    return NULL;
}
//GstPadEventFunction
#define GO(A)   \
static uintptr_t my_GstPadEventFunction_fct_##A = 0;                                        \
static int my_GstPadEventFunction_##A(void* a, void* b, void* c)                            \
{                                                                                           \
    return (int)RunFunctionFmt(my_GstPadEventFunction_fct_##A, "ppp", a, b, c); \
}
SUPER()
#undef GO
static void* findGstPadEventFunctionFct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my_GstPadEventFunction_fct_##A == (uintptr_t)fct) return my_GstPadEventFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstPadEventFunction_fct_##A == 0) {my_GstPadEventFunction_fct_##A = (uintptr_t)fct; return my_GstPadEventFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstPadEventFunction callback\n");
    return NULL;
}
//GstBusSyncHandler
#define GO(A)   \
static uintptr_t my_GstBusSyncHandler_fct_##A = 0;                                          \
static int my_GstBusSyncHandler_##A(void* a, void* b, void* c)                              \
{                                                                                           \
    return (int)RunFunctionFmt(my_GstBusSyncHandler_fct_##A, "ppp", a, b, c);   \
}
SUPER()
#undef GO
static void* findGstBusSyncHandlerFct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my_GstBusSyncHandler_fct_##A == (uintptr_t)fct) return my_GstBusSyncHandler_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstBusSyncHandler_fct_##A == 0) {my_GstBusSyncHandler_fct_##A = (uintptr_t)fct; return my_GstBusSyncHandler_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstBusSyncHandler callback\n");
    return NULL;
}

//GstPluginFeatureFilter
#define GO(A)   \
static uintptr_t my_GstPluginFeatureFilter_fct_##A = 0;                                     \
static int my_GstPluginFeatureFilter_##A(void* a, void* b)                                  \
{                                                                                           \
    return (int)RunFunctionFmt(my_GstPluginFeatureFilter_fct_##A, "pp", a, b);  \
}
SUPER()
#undef GO
static void* findGstPluginFeatureFilterFct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my_GstPluginFeatureFilter_fct_##A == (uintptr_t)fct) return my_GstPluginFeatureFilter_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstPluginFeatureFilter_fct_##A == 0) {my_GstPluginFeatureFilter_fct_##A = (uintptr_t)fct; return my_GstPluginFeatureFilter_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstPluginFeatureFilter callback\n");
    return NULL;
}

//GstCapsFilterMapFunc
#define GO(A)   \
static uintptr_t my_GstCapsFilterMapFunc_fct_##A = 0;                                       \
static int my_GstCapsFilterMapFunc_##A(void* a, void* b, void* c)                           \
{                                                                                           \
    return (int)RunFunctionFmt(my_GstCapsFilterMapFunc_fct_##A, "ppp", a, b, c);\
}
SUPER()
#undef GO
static void* findGstCapsFilterMapFuncFct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my_GstCapsFilterMapFunc_fct_##A == (uintptr_t)fct) return my_GstCapsFilterMapFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GstCapsFilterMapFunc_fct_##A == 0) {my_GstCapsFilterMapFunc_fct_##A = (uintptr_t)fct; return my_GstCapsFilterMapFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gstreamer GstCapsFilterMapFunc callback\n");
    return NULL;
}

#undef SUPER

EXPORT void my_gst_caps_set_simple(x86emu_t* emu, void* caps, void* field, void* b) {
    (void)emu;
    PREPARE_VALIST_(b);
    my->gst_caps_set_simple_valist(caps, field, VARARGS_(b));
}

EXPORT void my_gst_caps_set_simple_valist(x86emu_t* emu, void* caps, void* field, void* V) {
    (void)emu;
    PREPARE_VALIST_(V);
    my->gst_caps_set_simple_valist(caps, field, VARARGS_(V));
}

EXPORT int my_gst_structure_get(x86emu_t* emu, void* structure, void* field, void* b) {
    (void)emu;
    PREPARE_VALIST_(b);
    return my->gst_structure_get_valist(structure, field, VARARGS_(b));
}

EXPORT int my_gst_structure_get_valist(x86emu_t* emu, void* structure, void* field, void* V) {
    (void)emu;
    PREPARE_VALIST_(V);
    return my->gst_structure_get_valist(structure, field, VARARGS_(V));
}


EXPORT void my_gst_pad_set_activatemode_function_full(x86emu_t* emu, void* pad, void* f, void* data, void* d)
{
    my->gst_pad_set_activatemode_function_full(pad, findGstPadActivateModeFunctionFct(f), data, findDestroyFct(d));
}

EXPORT void my_gst_pad_set_query_function_full(x86emu_t* emu, void* pad, void* f, void* data, void* d)
{
    my->gst_pad_set_query_function_full(pad, findGstPadQueryFunctionFct(f), data, findDestroyFct(d));
}

EXPORT void my_gst_pad_set_getrange_function_full(x86emu_t* emu, void* pad, void* f, void* data, void* d)
{
    my->gst_pad_set_getrange_function_full(pad, findGstPadGetRangeFunctionFct(f), data, findDestroyFct(d));
}

EXPORT void my_gst_pad_set_chain_function_full(x86emu_t* emu, void* pad, void* f, void* data, void* d)
{
    my->gst_pad_set_chain_function_full(pad, findGstPadChainFunctionFct(f), data, findDestroyFct(d));
}

EXPORT void my_gst_pad_set_event_function_full(x86emu_t* emu, void* pad, void* f, void* data, void* d)
{
    my->gst_pad_set_event_function_full(pad, findGstPadEventFunctionFct(f), data, findDestroyFct(d));
}

EXPORT void my_gst_bus_set_sync_handler(x86emu_t* emu, void* bus, void* f, void* data, void* d)
{
    my->gst_bus_set_sync_handler(bus, findGstBusSyncHandlerFct(f), data, findDestroyFct(d));
}

EXPORT void* my_gst_buffer_new_wrapped_full(x86emu_t* emu, uint32_t f, void* data, size_t maxsize, size_t offset, size_t size, void* user, void* d)
{
    return my->gst_buffer_new_wrapped_full(f, data, maxsize, offset, size, user, findDestroyFct(d));
}

EXPORT void my_gst_mini_object_set_qdata(x86emu_t* emu, void* object, void* quark, void* data, void* d)
{
    PREPARE_VALIST_(V);
    my->gst_mini_object_set_qdata(object, quark, data, findDestroyFct(d));
}

EXPORT void* my_gst_caps_new_simple(x86emu_t* emu, void* type, void* name, void* V)
{
    // need to unroll the function here, there is no direct VA equivalent
    void* caps = my->gst_caps_new_empty();
    void* structure = my->gst_structure_new_valist(type, name, VARARGS_(V));
    if (structure)
        my->gst_caps_append_structure(caps, structure);
    else
        my->gst_caps_replace(&caps, NULL);

    return caps;
}

EXPORT void* my_gst_registry_feature_filter(x86emu_t* emu, void* reg, void* filter, int first, void* data)
{
    return my->gst_registry_feature_filter(reg, findGstPluginFeatureFilterFct(filter), first, data);
}

EXPORT int my_gst_caps_foreach(x86emu_t* emu, void* caps, void* f, void* data)
{
    return my->gst_caps_foreach(caps, findGstCapsFilterMapFuncFct(f), data);
}

#define PRE_INIT    \
    if(box86_nogtk) \
        return -1;

#ifdef ANDROID
    #define CUSTOM_INIT \
        getMy(lib);     \
        SetGstObjectID(my->gst_object_get_type());                 \
        SetGstAllocatorID(my->gst_allocator_get_type());           \
        setNeededLibs(lib, 1, "libgtk-3.so");
#else
    #define CUSTOM_INIT \
        getMy(lib);     \
        SetGstObjectID(my->gst_object_get_type());                 \
        SetGstAllocatorID(my->gst_allocator_get_type());           \
        setNeededLibs(lib, 1, "libgtk-3.so.0");
#endif

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"

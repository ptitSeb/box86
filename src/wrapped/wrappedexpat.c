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
#include "emu/x86emu_private.h"
#include "callback.h"
#include "librarian.h"
#include "box86context.h"
#include "emu/x86emu_private.h"
#include "myalign.h"

const char* expatName = "libexpat.so.1";
#define LIBNAME expat

#include "generated/wrappedexpattypes.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// Start ...
#define GO(A)   \
static uintptr_t my_Start_fct_##A = 0;                                                  \
static void* my_Start_##A(void* data, void* name, void* attr)                           \
{                                                                                       \
    return (void*)RunFunctionFmt(my_Start_fct_##A, "ppp", data, name, attr);\
}
SUPER()
#undef GO
static void* find_Start_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_Start_fct_##A == (uintptr_t)fct) return my_Start_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_Start_fct_##A == 0) {my_Start_fct_##A = (uintptr_t)fct; return my_Start_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for expat Start callback\n");
    return NULL;
}
// End ...
#define GO(A)   \
static uintptr_t my_End_fct_##A = 0;                                \
static void my_End_##A(void* data, void* name)                      \
{                                                                   \
    RunFunctionFmt(my_End_fct_##A, "pp", data, name);   \
}
SUPER()
#undef GO
static void* find_End_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_End_fct_##A == (uintptr_t)fct) return my_End_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_End_fct_##A == 0) {my_End_fct_##A = (uintptr_t)fct; return my_End_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for expat End callback\n");
    return NULL;
}
// CharData ...
#define GO(A)   \
static uintptr_t my_CharData_fct_##A = 0;                               \
static void my_CharData_##A(void* data, void* s, int l)                 \
{                                                                       \
    RunFunctionFmt(my_CharData_fct_##A, "ppi", data, s, l); \
}
SUPER()
#undef GO
static void* find_CharData_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_CharData_fct_##A == (uintptr_t)fct) return my_CharData_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_CharData_fct_##A == 0) {my_CharData_fct_##A = (uintptr_t)fct; return my_CharData_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for expat CharData callback\n");
    return NULL;
}
// StartDoctypeDeclHandler
#define GO(A)   \
static uintptr_t my_StartDoctypeDeclHandler_fct_##A = 0;                                                  \
static void* my_StartDoctypeDeclHandler_##A(void* data, void* name, void* sysid, void* pubid, int has_internal_subset) \
{                                                                                       \
    return (void*)RunFunctionFmt(my_StartDoctypeDeclHandler_fct_##A, "ppppi", data, name, sysid, pubid, has_internal_subset);\
}
SUPER()
#undef GO
static void* find_StartDoctypeDeclHandler_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_StartDoctypeDeclHandler_fct_##A == (uintptr_t)fct) return my_StartDoctypeDeclHandler_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_StartDoctypeDeclHandler_fct_##A == 0) {my_StartDoctypeDeclHandler_fct_##A = (uintptr_t)fct; return my_StartDoctypeDeclHandler_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for expat StartDoctypeDeclHandler callback\n");
    return NULL;
}
// EndDoctypeDeclHandler
#define GO(A)   \
static uintptr_t my_EndDoctypeDeclHandler_fct_##A = 0;                                                  \
static void* my_EndDoctypeDeclHandler_##A(void* data) \
{                                                                                       \
    return (void*)RunFunctionFmt(my_EndDoctypeDeclHandler_fct_##A, "p", data);\
}
SUPER()
#undef GO
static void* find_EndDoctypeDeclHandler_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_EndDoctypeDeclHandler_fct_##A == (uintptr_t)fct) return my_EndDoctypeDeclHandler_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_EndDoctypeDeclHandler_fct_##A == 0) {my_EndDoctypeDeclHandler_fct_##A = (uintptr_t)fct; return my_EndDoctypeDeclHandler_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for expat EndDoctypeDeclHandler callback\n");
    return NULL;
}
#undef SUPER

EXPORT void my_XML_SetElementHandler(x86emu_t* emu, void* p, void* start, void* end)
{
    (void)emu;
    my->XML_SetElementHandler(p, find_Start_Fct(start), find_End_Fct(end));
}

EXPORT void my_XML_SetDoctypeDeclHandler(x86emu_t* emu, void* p, void* start, void* end)
{
    (void)emu;
    my->XML_SetDoctypeDeclHandler(p, find_StartDoctypeDeclHandler_Fct(start), find_EndDoctypeDeclHandler_Fct(end));
}

EXPORT void my_XML_SetCharacterDataHandler(x86emu_t* emu, void* p, void* h)
{
    (void)emu;
    my->XML_SetCharacterDataHandler(p, find_CharData_Fct(h));
}

#define CUSTOM_INIT \
    getMy(lib);

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"

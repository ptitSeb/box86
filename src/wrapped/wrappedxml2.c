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
#include "box86context.h"
#include "librarian.h"
#include "callback.h"

const char* xml2Name = "libxml2.so.2";
#define LIBNAME xml2
static library_t *my_lib = NULL;

typedef void*   (*pFv_t)        ();
typedef void    (*vFp_t)        (void*);
typedef void*   (*pFpp_t)       (void*, void*);
typedef void    (*vFpp_t)       (void*, void*);
typedef int     (*iFppp_t)      (void*, void*, void*);
typedef void    (*vFppp_t)      (void*, void*, void*);
typedef void*   (*pFppp_t)      (void*, void*, void*);
typedef void*   (*pFpppi_t)     (void*, void*, void*, int);
typedef int     (*iFpppp_t)     (void*, void*, void*, void*);
typedef void    (*vFpppp_t)     (void*, void*, void*, void*);
typedef void*   (*pFpppp_t)     (void*, void*, void*, void*);
typedef void*   (*pFppppi_t)    (void*, void*, void*, void*, int);
typedef int     (*iFppppp_t)    (void*, void*, void*, void*, void*);
typedef void    (*vFppppp_t)    (void*, void*, void*, void*, void*);
typedef int     (*iFpppppp_t)   (void*, void*, void*, void*, void*, void*);
typedef void    (*vFpppppp_t)   (void*, void*, void*, void*, void*, void*);

#define SUPER()                                     \
    GO(xmlHashCopy, pFpp_t)                         \
    GO(xmlHashFree, vFpp_t)                         \
    GO(xmlHashRemoveEntry, iFppp_t)                 \
    GO(xmlHashRemoveEntry2, iFpppp_t)               \
    GO(xmlHashRemoveEntry3, iFppppp_t)              \
    GO(xmlHashScan, vFppp_t)                        \
    GO(xmlHashScan3, vFpppppp_t)                    \
    GO(xmlHashScanFull, vFppp_t)                    \
    GO(xmlHashScanFull3, vFpppppp_t)                \
    GO(xmlHashUpdateEntry, iFpppp_t)                \
    GO(xmlHashUpdateEntry2, iFppppp_t)              \
    GO(xmlHashUpdateEntry3, iFpppppp_t)             \
    GO(xmlGetExternalEntityLoader, pFv_t)           \
    GO(xmlNewCharEncodingHandler, pFppp_t)          \
    GO(xmlOutputBufferCreateIO, pFpppp_t)           \
    GO(xmlRegisterInputCallbacks, iFpppp_t)         \
    GO(xmlSaveToIO, pFppppi_t)                      \
    GO(xmlSchemaSetParserErrors, vFpppp_t)          \
    GO(xmlSchemaSetParserStructuredErrors, vFppp_t) \
    GO(xmlSchemaSetValidErrors, vFpppp_t)           \
    GO(xmlSchemaSetValidStructuredErrors, vFppp_t)  \
    GO(xmlSetExternalEntityLoader, vFp_t)           \
    GO(xmlXPathRegisterFunc, iFppp_t)               \
    GO(xmlParserInputBufferCreateIO, pFpppi_t)      \

typedef struct xml2_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} xml2_my_t;

void* getXml2My(library_t* lib)
{
    xml2_my_t* my = (xml2_my_t*)calloc(1, sizeof(xml2_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeXml2My(void* lib)
{
    //xml2_my_t *my = (xml2_my_t *)lib;
}

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// xmlHashCopier ...
#define GO(A)   \
static uintptr_t my_xmlHashCopier_fct_##A = 0;                                  \
static void* my_xmlHashCopier_##A(void* a, void* b)                             \
{                                                                               \
    return (void*)RunFunction(my_context, my_xmlHashCopier_fct_##A, 2, a, b);   \
}
SUPER()
#undef GO
static void* find_xmlHashCopier_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlHashCopier_fct_##A == (uintptr_t)fct) return my_xmlHashCopier_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlHashCopier_fct_##A == 0) {my_xmlHashCopier_fct_##A = (uintptr_t)fct; return my_xmlHashCopier_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlHashCopier callback\n");
    return NULL;
}
// xmlHashDeallocator ...
#define GO(A)   \
static uintptr_t my_xmlHashDeallocator_fct_##A = 0;                     \
static void my_xmlHashDeallocator_##A(void* a, void* b)                 \
{                                                                       \
    RunFunction(my_context, my_xmlHashDeallocator_fct_##A, 2, a, b);    \
}
SUPER()
#undef GO
static void* find_xmlHashDeallocator_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlHashDeallocator_fct_##A == (uintptr_t)fct) return my_xmlHashDeallocator_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlHashDeallocator_fct_##A == 0) {my_xmlHashDeallocator_fct_##A = (uintptr_t)fct; return my_xmlHashDeallocator_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlHashDeallocator callback\n");
    return NULL;
}
// xmlHashScanner ...
#define GO(A)   \
static uintptr_t my_xmlHashScanner_fct_##A = 0;                     \
static void my_xmlHashScanner_##A(void* a, void* b, void* c)        \
{                                                                   \
    RunFunction(my_context, my_xmlHashScanner_fct_##A, 3, a, b, c); \
}
SUPER()
#undef GO
static void* find_xmlHashScanner_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlHashScanner_fct_##A == (uintptr_t)fct) return my_xmlHashScanner_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlHashScanner_fct_##A == 0) {my_xmlHashScanner_fct_##A = (uintptr_t)fct; return my_xmlHashScanner_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlHashScanner callback\n");
    return NULL;
}
// xmlHashScannerFull ...
#define GO(A)   \
static uintptr_t my_xmlHashScannerFull_fct_##A = 0;                                 \
static void my_xmlHashScannerFull_##A(void* a, void* b, void* c, void* c2, void* c3)\
{                                                                                   \
    RunFunction(my_context, my_xmlHashScannerFull_fct_##A, 5, a, b, c, c2, c3);     \
}
SUPER()
#undef GO
static void* find_xmlHashScannerFull_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlHashScannerFull_fct_##A == (uintptr_t)fct) return my_xmlHashScannerFull_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlHashScannerFull_fct_##A == 0) {my_xmlHashScannerFull_fct_##A = (uintptr_t)fct; return my_xmlHashScannerFull_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlHashScannerFull callback\n");
    return NULL;
}
// xmlCharEncodingInputFunc ...
#define GO(A)   \
static uintptr_t my_xmlCharEncodingInputFunc_fct_##A = 0;                               \
static int my_xmlCharEncodingInputFunc_##A(void* a, void* b, void* c, void* d)          \
{                                                                                       \
    return RunFunction(my_context, my_xmlCharEncodingInputFunc_fct_##A, 4, a, b, c, d); \
}
SUPER()
#undef GO
static void* find_xmlCharEncodingInputFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlCharEncodingInputFunc_fct_##A == (uintptr_t)fct) return my_xmlCharEncodingInputFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlCharEncodingInputFunc_fct_##A == 0) {my_xmlCharEncodingInputFunc_fct_##A = (uintptr_t)fct; return my_xmlCharEncodingInputFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlCharEncodingInputFunc callback\n");
    return NULL;
}
// xmlCharEncodingOutputFunc ...
#define GO(A)   \
static uintptr_t my_xmlCharEncodingOutputFunc_fct_##A = 0;                              \
static int my_xmlCharEncodingOutputFunc_##A(void* a, void* b, void* c, void* d)         \
{                                                                                       \
    return RunFunction(my_context, my_xmlCharEncodingOutputFunc_fct_##A, 4, a, b, c, d);\
}
SUPER()
#undef GO
static void* find_xmlCharEncodingOutputFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlCharEncodingOutputFunc_fct_##A == (uintptr_t)fct) return my_xmlCharEncodingOutputFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlCharEncodingOutputFunc_fct_##A == 0) {my_xmlCharEncodingOutputFunc_fct_##A = (uintptr_t)fct; return my_xmlCharEncodingOutputFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlCharEncodingOutputFunc callback\n");
    return NULL;
}
// xmlOutputWriteCallback ...
#define GO(A)   \
static uintptr_t my_xmlOutputWriteCallback_fct_##A = 0;                             \
static int my_xmlOutputWriteCallback_##A(void* a, void* b, int c)                   \
{                                                                                   \
    return RunFunction(my_context, my_xmlOutputWriteCallback_fct_##A, 3, a, b, c);  \
}
SUPER()
#undef GO
static void* find_xmlOutputWriteCallback_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlOutputWriteCallback_fct_##A == (uintptr_t)fct) return my_xmlOutputWriteCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlOutputWriteCallback_fct_##A == 0) {my_xmlOutputWriteCallback_fct_##A = (uintptr_t)fct; return my_xmlOutputWriteCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlOutputWriteCallback callback\n");
    return NULL;
}
// xmlOutputCloseCallback ...
#define GO(A)   \
static uintptr_t my_xmlOutputCloseCallback_fct_##A = 0;                         \
static int my_xmlOutputCloseCallback_##A(void* a)                               \
{                                                                               \
    return RunFunction(my_context, my_xmlOutputCloseCallback_fct_##A, 1, a);    \
}
SUPER()
#undef GO
static void* find_xmlOutputCloseCallback_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlOutputCloseCallback_fct_##A == (uintptr_t)fct) return my_xmlOutputCloseCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlOutputCloseCallback_fct_##A == 0) {my_xmlOutputCloseCallback_fct_##A = (uintptr_t)fct; return my_xmlOutputCloseCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlOutputCloseCallback callback\n");
    return NULL;
}
// xmlInputMatchCallback ...
#define GO(A)   \
static uintptr_t my_xmlInputMatchCallback_fct_##A = 0;                      \
static int my_xmlInputMatchCallback_##A(void* a)                            \
{                                                                           \
    return RunFunction(my_context, my_xmlInputMatchCallback_fct_##A, 1, a); \
}
SUPER()
#undef GO
static void* find_xmlInputMatchCallback_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlInputMatchCallback_fct_##A == (uintptr_t)fct) return my_xmlInputMatchCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlInputMatchCallback_fct_##A == 0) {my_xmlInputMatchCallback_fct_##A = (uintptr_t)fct; return my_xmlInputMatchCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlInputMatchCallback callback\n");
    return NULL;
}
// xmlInputOpenCallback ...
#define GO(A)   \
static uintptr_t my_xmlInputOpenCallback_fct_##A = 0;                               \
static void* my_xmlInputOpenCallback_##A(void* a)                                   \
{                                                                                   \
    return (void*)RunFunction(my_context, my_xmlInputOpenCallback_fct_##A, 1, a);   \
}
SUPER()
#undef GO
static void* find_xmlInputOpenCallback_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlInputOpenCallback_fct_##A == (uintptr_t)fct) return my_xmlInputOpenCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlInputOpenCallback_fct_##A == 0) {my_xmlInputOpenCallback_fct_##A = (uintptr_t)fct; return my_xmlInputOpenCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlInputOpenCallback callback\n");
    return NULL;
}
// xmlInputReadCallback ...
#define GO(A)   \
static uintptr_t my_xmlInputReadCallback_fct_##A = 0;                               \
static int my_xmlInputReadCallback_##A(void* a, void* b, int c)                     \
{                                                                                   \
    return RunFunction(my_context, my_xmlInputReadCallback_fct_##A, 3, a, b, c);    \
}
SUPER()
#undef GO
static void* find_xmlInputReadCallback_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlInputReadCallback_fct_##A == (uintptr_t)fct) return my_xmlInputReadCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlInputReadCallback_fct_##A == 0) {my_xmlInputReadCallback_fct_##A = (uintptr_t)fct; return my_xmlInputReadCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlInputReadCallback callback\n");
    return NULL;
}
// xmlInputCloseCallback ...
#define GO(A)   \
static uintptr_t my_xmlInputCloseCallback_fct_##A = 0;                         \
static int my_xmlInputCloseCallback_##A(void* a)                               \
{                                                                               \
    return RunFunction(my_context, my_xmlInputCloseCallback_fct_##A, 1, a);    \
}
SUPER()
#undef GO
static void* find_xmlInputCloseCallback_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlInputCloseCallback_fct_##A == (uintptr_t)fct) return my_xmlInputCloseCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlInputCloseCallback_fct_##A == 0) {my_xmlInputCloseCallback_fct_##A = (uintptr_t)fct; return my_xmlInputCloseCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlInputCloseCallback callback\n");
    return NULL;
}
// xmlSchemaValidityErrorFunc (use a VA)...
#define GO(A)   \
static uintptr_t my_xmlSchemaValidityErrorFunc_fct_##A = 0;                                                                     \
static void my_xmlSchemaValidityErrorFunc_##A(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i)  \
{                                                                                                                               \
    RunFunction(my_context, my_xmlSchemaValidityErrorFunc_fct_##A, 9, a, b, c, d, e, f, g, h, i);                               \
}
SUPER()
#undef GO
static void* find_xmlSchemaValidityErrorFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlSchemaValidityErrorFunc_fct_##A == (uintptr_t)fct) return my_xmlSchemaValidityErrorFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlSchemaValidityErrorFunc_fct_##A == 0) {my_xmlSchemaValidityErrorFunc_fct_##A = (uintptr_t)fct; return my_xmlSchemaValidityErrorFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlSchemaValidityErrorFunc callback\n");
    return NULL;
}
// xmlSchemaValidityWarningFunc (use a VA)...
#define GO(A)   \
static uintptr_t my_xmlSchemaValidityWarningFunc_fct_##A = 0;                                                                       \
static void my_xmlSchemaValidityWarningFunc_##A(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i)    \
{                                                                                                                                   \
    RunFunction(my_context, my_xmlSchemaValidityWarningFunc_fct_##A, 9, a, b, c, d, e, f, g, h, i);                                 \
}
SUPER()
#undef GO
static void* find_xmlSchemaValidityWarningFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlSchemaValidityWarningFunc_fct_##A == (uintptr_t)fct) return my_xmlSchemaValidityWarningFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlSchemaValidityWarningFunc_fct_##A == 0) {my_xmlSchemaValidityWarningFunc_fct_##A = (uintptr_t)fct; return my_xmlSchemaValidityWarningFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlSchemaValidityWarningFunc callback\n");
    return NULL;
}
// xmlStructuredErrorFunc ...
#define GO(A)   \
static uintptr_t my_xmlStructuredErrorFunc_fct_##A = 0;                     \
static void my_xmlStructuredErrorFunc_##A(void* a, void* b)                 \
{                                                                           \
    RunFunction(my_context, my_xmlStructuredErrorFunc_fct_##A, 2, a, b);    \
}
SUPER()
#undef GO
static void* find_xmlStructuredErrorFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlStructuredErrorFunc_fct_##A == (uintptr_t)fct) return my_xmlStructuredErrorFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlStructuredErrorFunc_fct_##A == 0) {my_xmlStructuredErrorFunc_fct_##A = (uintptr_t)fct; return my_xmlStructuredErrorFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlStructuredErrorFunc callback\n");
    return NULL;
}
// xmlXPathFunction ...
#define GO(A)   \
static uintptr_t my_xmlXPathFunction_fct_##A = 0;                   \
static void my_xmlXPathFunction_##A(void* a, int b)                 \
{                                                                   \
    RunFunction(my_context, my_xmlXPathFunction_fct_##A, 2, a, b);  \
}
SUPER()
#undef GO
static void* find_xmlXPathFunction_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlXPathFunction_fct_##A == (uintptr_t)fct) return my_xmlXPathFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlXPathFunction_fct_##A == 0) {my_xmlXPathFunction_fct_##A = (uintptr_t)fct; return my_xmlXPathFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlXPathFunction callback\n");
    return NULL;
}

// xmlExternalEntityLoader
#define GO(A)   \
static uintptr_t my_xmlExternalEntityLoader_fct_##A = 0;                                \
static void* my_xmlExternalEntityLoader_##A(void* a, void* b, void* c)                  \
{                                                                                       \
    return (void*)RunFunction(my_context, my_xmlHashScannerFull_fct_##A, 3, a, b, c);   \
}
SUPER()
#undef GO
static void* find_xmlExternalEntityLoaderFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xmlExternalEntityLoader_fct_##A == (uintptr_t)fct) return my_xmlExternalEntityLoader_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xmlExternalEntityLoader_fct_##A == 0) {my_xmlExternalEntityLoader_fct_##A = (uintptr_t)fct; return my_xmlExternalEntityLoader_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libxml2 xmlExternalEntityLoader callback\n");
    return NULL;
}
static void* reverse_xmlExternalEntityLoaderFct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->priv.w.bridge, fct))
        return (void*)CheckBridged(my_lib->priv.w.bridge, fct);
    #define GO(A) if(my_xmlExternalEntityLoader_##A == fct) return (void*)my_xmlExternalEntityLoader_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->priv.w.bridge, pFppp, fct, 0);
}


#undef SUPER

EXPORT void* my_xmlHashCopy(x86emu_t* emu, void* table, void* f)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    return my->xmlHashCopy(table, find_xmlHashCopier_Fct(f));
}

EXPORT void my_xmlHashFree(x86emu_t* emu, void* table, void* f)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    my->xmlHashFree(table, find_xmlHashDeallocator_Fct(f));
}

EXPORT int my_xmlHashRemoveEntry(x86emu_t* emu, void* table, void* name, void* f)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    return my->xmlHashRemoveEntry(table, name, find_xmlHashDeallocator_Fct(f));
}
EXPORT int my_xmlHashRemoveEntry2(x86emu_t* emu, void* table, void* name, void* name2, void* f)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    return my->xmlHashRemoveEntry2(table, name, name2, find_xmlHashDeallocator_Fct(f));
}
EXPORT int my_xmlHashRemoveEntry3(x86emu_t* emu, void* table, void* name, void* name2, void* name3, void* f)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    return my->xmlHashRemoveEntry3(table, name, name2, name3, find_xmlHashDeallocator_Fct(f));
}

EXPORT void my_xmlHashScan(x86emu_t* emu, void* table, void* f, void* data)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    my->xmlHashScan(table, find_xmlHashScanner_Fct(f), data);
}
EXPORT void my_xmlHashScan3(x86emu_t* emu, void* table, void* name, void* name2, void* name3, void* f, void* data)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    my->xmlHashScan3(table, name, name2, name3, find_xmlHashScanner_Fct(f), data);
}
EXPORT void my_xmlHashScanFull(x86emu_t* emu, void* table, void* f, void* data)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    my->xmlHashScanFull(table, find_xmlHashScannerFull_Fct(f), data);
}
EXPORT void my_xmlHashScanFull3(x86emu_t* emu, void* table, void* name, void* name2, void* name3, void* f, void* data)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    my->xmlHashScanFull3(table, name, name2, name3, find_xmlHashScannerFull_Fct(f), data);
}

EXPORT int my_xmlHashUpdateEntry(x86emu_t* emu, void* table, void* name, void* data, void* f)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    return my->xmlHashUpdateEntry(table, name, data, find_xmlHashDeallocator_Fct(f));
}
EXPORT int my_xmlHashUpdateEntry2(x86emu_t* emu, void* table, void* name, void* name2, void* data, void* f)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    return my->xmlHashUpdateEntry2(table, name, name2, data, find_xmlHashDeallocator_Fct(f));
}
EXPORT int my_xmlHashUpdateEntry3(x86emu_t* emu, void* table, void* name, void* name2, void* name3, void* data, void* f)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    return my->xmlHashUpdateEntry3(table, name, name2, name3, data, find_xmlHashDeallocator_Fct(f));
}

EXPORT void* my_xmlGetExternalEntityLoader(x86emu_t* emu)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    return reverse_xmlExternalEntityLoaderFct(my->xmlGetExternalEntityLoader());
}

EXPORT void* my_xmlNewCharEncodingHandler(x86emu_t* emu, void* name, void* fin, void* fout)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    return my->xmlNewCharEncodingHandler(name, find_xmlCharEncodingInputFunc_Fct(fin), find_xmlCharEncodingOutputFunc_Fct(fout));
}

EXPORT void* my_xmlOutputBufferCreateIO(x86emu_t* emu, void* fwrite, void* fclose, void* ioctx, void* encoder)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    return my->xmlOutputBufferCreateIO(find_xmlOutputWriteCallback_Fct(fwrite), find_xmlOutputCloseCallback_Fct(fclose), ioctx, encoder);
}

EXPORT int my_xmlRegisterInputCallbacks(x86emu_t* emu, void* fmatch, void* fop, void* frd, void* fcl)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    return my->xmlRegisterInputCallbacks(find_xmlInputMatchCallback_Fct(fmatch), find_xmlInputOpenCallback_Fct(fop), find_xmlInputReadCallback_Fct(frd), find_xmlOutputCloseCallback_Fct(fcl));
}

EXPORT void* my_xmlSaveToIO(x86emu_t* emu, void* fwrt, void* fcl, void* ioctx, void* encoding, int options)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    return my->xmlSaveToIO(find_xmlOutputWriteCallback_Fct(fwrt), find_xmlOutputCloseCallback_Fct(fcl), ioctx, encoding, options);
}

EXPORT void my_xmlSchemaSetParserErrors(x86emu_t* emu, void* ctxt, void* ferr, void* fwarn, void* ctx)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    my->xmlSchemaSetParserErrors(ctxt, find_xmlSchemaValidityErrorFunc_Fct(ferr), find_xmlSchemaValidityWarningFunc_Fct(fwarn), ctx);
}

EXPORT void my_xmlSchemaSetParserStructuredErrors(x86emu_t* emu, void* ctxt, void* ferr, void* ctx)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    my->xmlSchemaSetParserStructuredErrors(ctxt, find_xmlStructuredErrorFunc_Fct(ferr), ctx);
}

EXPORT void my_xmlSchemaSetValidErrors(x86emu_t* emu, void* ctxt, void* ferr, void* fwarn, void* ctx)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    my->xmlSchemaSetValidErrors(ctxt, find_xmlSchemaValidityErrorFunc_Fct(ferr), find_xmlSchemaValidityWarningFunc_Fct(fwarn), ctx);
}

EXPORT void my_xmlSchemaSetValidStructuredErrors(x86emu_t* emu, void* ctxt, void* ferr, void* ctx)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    my->xmlSchemaSetValidStructuredErrors(ctxt, find_xmlStructuredErrorFunc_Fct(ferr), ctx);
}

EXPORT void my_xmlSetExternalEntityLoader(x86emu_t* emu, void* f)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    my->xmlSetExternalEntityLoader(find_xmlExternalEntityLoaderFct(f));
}

EXPORT int my_xmlXPathRegisterFunc(x86emu_t* emu, void* ctxt, void* name, void* f)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    return my->xmlXPathRegisterFunc(ctxt, name, find_xmlXPathFunction_Fct(f));
}

EXPORT void* my_xmlParserInputBufferCreateIO(x86emu_t* emu, void* ioread, void* ioclose, void* ioctx, int enc)
{
    xml2_my_t* my = (xml2_my_t*)my_lib->priv.w.p2;

    return my->xmlParserInputBufferCreateIO(find_xmlInputReadCallback_Fct(ioread), find_xmlInputCloseCallback_Fct(ioclose), ioctx, enc);
}

#define CUSTOM_INIT \
    lib->priv.w.p2 = getXml2My(lib);    \
    my_lib = lib;

#define CUSTOM_FINI \
    freeXml2My(lib->priv.w.p2); \
    free(lib->priv.w.p2);       \
    my_lib = NULL;

#include "wrappedlib_init.h"

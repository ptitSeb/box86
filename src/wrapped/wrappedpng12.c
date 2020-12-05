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

const char* png12Name = "libpng12.so.0";
#define LIBNAME png12

typedef void  (*vFpp_t)(void*, void*);
typedef void  (*vFppp_t)(void*, void*, void*);
typedef void  (*vFpppp_t)(void*, void*, void*, void*);
typedef void* (*pFpppp_t)(void*, void*, void*, void*);
typedef void  (*vFppppp_t)(void*, void*, void*, void*, void*);
typedef void* (*pFppppppp_t)(void*, void*, void*, void*, void*, void*, void*);

#define SUPER() \
    GO(png_set_write_fn, vFpppp_t)              \
    GO(png_set_read_fn, vFppp_t)                \
    GO(png_set_error_fn, vFpppp_t)              \
    GO(png_create_read_struct_2, pFppppppp_t)   \
    GO(png_create_write_struct_2, pFppppppp_t)  \
    GO(png_set_progressive_read_fn, vFppppp_t)  \
    GO(png_create_read_struct, pFpppp_t)        \

typedef struct png12_my_s {
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
    // functions
} png12_my_t;

void* getPng12My(library_t* lib)
{
    png12_my_t* my = (png12_my_t*)calloc(1, sizeof(png12_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freePng12My(void* lib)
{
    //png12_my_t *my = (png12_my_t *)lib;
}

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)

// user_write
#define GO(A)   \
static uintptr_t my_user_write_fct_##A = 0;   \
static void my_user_write_##A(void* png_ptr, void* data, int32_t length)    \
{                                       \
    RunFunction(my_context, my_user_write_fct_##A, 3, png_ptr, data, length);\
}
SUPER()
#undef GO
static void* finduser_writeFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_user_write_fct_##A == (uintptr_t)fct) return my_user_write_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_user_write_fct_##A == 0) {my_user_write_fct_##A = (uintptr_t)fct; return my_user_write_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libpng12 user_write callback\n");
    return NULL;
}
// user_flush
#define GO(A)   \
static uintptr_t my_user_flush_fct_##A = 0;   \
static void my_user_flush_##A(void* png_ptr)    \
{                                       \
    RunFunction(my_context, my_user_flush_fct_##A, 1, png_ptr);\
}
SUPER()
#undef GO
static void* finduser_flushFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_user_flush_fct_##A == (uintptr_t)fct) return my_user_flush_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_user_flush_fct_##A == 0) {my_user_flush_fct_##A = (uintptr_t)fct; return my_user_flush_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libpng12 user_flush callback\n");
    return NULL;
}
// user_read
#define GO(A)   \
static uintptr_t my_user_read_fct_##A = 0;   \
static void my_user_read_##A(void* png_ptr, void* data, int32_t length)    \
{                                       \
    RunFunction(my_context, my_user_read_fct_##A, 3, png_ptr, data, length);\
}
SUPER()
#undef GO
static void* finduser_readFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_user_read_fct_##A == (uintptr_t)fct) return my_user_read_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_user_read_fct_##A == 0) {my_user_read_fct_##A = (uintptr_t)fct; return my_user_read_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libpng12 user_read callback\n");
    return NULL;
}
// error
#define GO(A)   \
static uintptr_t my_error_fct_##A = 0;   \
static void my_error_##A(void* a, void* b)    \
{                                       \
    RunFunction(my_context, my_error_fct_##A, 2, a, b);\
}
SUPER()
#undef GO
static void* finderrorFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_error_fct_##A == (uintptr_t)fct) return my_error_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_error_fct_##A == 0) {my_error_fct_##A = (uintptr_t)fct; return my_error_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libpng12 error callback\n");
    return NULL;
}
// warning
#define GO(A)   \
static uintptr_t my_warning_fct_##A = 0;   \
static void my_warning_##A(void* a, void* b)    \
{                                       \
    RunFunction(my_context, my_warning_fct_##A, 2, a, b);\
}
SUPER()
#undef GO
static void* findwarningFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_warning_fct_##A == (uintptr_t)fct) return my_warning_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_warning_fct_##A == 0) {my_warning_fct_##A = (uintptr_t)fct; return my_warning_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libpng12 warning callback\n");
    return NULL;
}
// malloc
#define GO(A)   \
static uintptr_t my_malloc_fct_##A = 0;   \
static void my_malloc_##A(void* a, unsigned long b)    \
{                                       \
    RunFunction(my_context, my_malloc_fct_##A, 2, a, b);\
}
SUPER()
#undef GO
static void* findmallocFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_malloc_fct_##A == (uintptr_t)fct) return my_malloc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_malloc_fct_##A == 0) {my_malloc_fct_##A = (uintptr_t)fct; return my_malloc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libpng12 malloc callback\n");
    return NULL;
}
// free
#define GO(A)   \
static uintptr_t my_free_fct_##A = 0;   \
static void my_free_##A(void* a, void* b)    \
{                                       \
    RunFunction(my_context, my_free_fct_##A, 2, a, b);\
}
SUPER()
#undef GO
static void* findfreeFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_free_fct_##A == (uintptr_t)fct) return my_free_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_free_fct_##A == 0) {my_free_fct_##A = (uintptr_t)fct; return my_free_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libpng12 free callback\n");
    return NULL;
}

// progressive_info
#define GO(A)   \
static uintptr_t my_progressive_info_fct_##A = 0;   \
static void my_progressive_info_##A(void* a, void* b)    \
{                                       \
    RunFunction(my_context, my_progressive_info_fct_##A, 2, a, b);\
}
SUPER()
#undef GO
static void* findprogressive_infoFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_progressive_info_fct_##A == (uintptr_t)fct) return my_progressive_info_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_progressive_info_fct_##A == 0) {my_progressive_info_fct_##A = (uintptr_t)fct; return my_progressive_info_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libpng12 progressive_info callback\n");
    return NULL;
}

// progressive_end
#define GO(A)   \
static uintptr_t my_progressive_end_fct_##A = 0;   \
static void my_progressive_end_##A(void* a, void* b)    \
{                                       \
    RunFunction(my_context, my_progressive_end_fct_##A, 2, a, b);\
}
SUPER()
#undef GO
static void* findprogressive_endFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_progressive_end_fct_##A == (uintptr_t)fct) return my_progressive_end_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_progressive_end_fct_##A == 0) {my_progressive_end_fct_##A = (uintptr_t)fct; return my_progressive_end_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libpng12 progressive_end callback\n");
    return NULL;
}

// progressive_row
#define GO(A)   \
static uintptr_t my_progressive_row_fct_##A = 0;   \
static void my_progressive_row_##A(void* a, void* b, uint32_t c, int d)    \
{                                       \
    RunFunction(my_context, my_progressive_row_fct_##A, 4, a, b, c, d);\
}
SUPER()
#undef GO
static void* findprogressive_rowFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_progressive_row_fct_##A == (uintptr_t)fct) return my_progressive_row_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_progressive_row_fct_##A == 0) {my_progressive_row_fct_##A = (uintptr_t)fct; return my_progressive_row_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libpng12 progressive_row callback\n");
    return NULL;
}

#undef SUPER

EXPORT void my12_png_set_write_fn(x86emu_t* emu, void* png_ptr, void* ioptr, void* write_fn, void* flush_fn)
{
    library_t * lib = GetLibInternal(png12Name);
    png12_my_t *my = (png12_my_t*)lib->priv.w.p2;

    my->png_set_write_fn(png_ptr, ioptr, finduser_writeFct(write_fn), finduser_flushFct(flush_fn));
}

EXPORT void my12_png_set_read_fn(x86emu_t* emu, void* png_ptr, void* ioptr, void* read_fn)
{
    library_t * lib = GetLibInternal(png12Name);
    png12_my_t *my = (png12_my_t*)lib->priv.w.p2;

    my->png_set_read_fn(png_ptr, ioptr, finduser_readFct(read_fn));
}

EXPORT void my12_png_set_error_fn(x86emu_t* emu, void* pngptr, void* errorptr, void* error_fn, void* warning_fn)
{
    library_t * lib = GetLibInternal(png12Name);
    png12_my_t *my = (png12_my_t*)lib->priv.w.p2;

    my->png_set_error_fn(pngptr, errorptr, finderrorFct(error_fn), findwarningFct(warning_fn));
}

EXPORT void* my12_png_create_read_struct_2(x86emu_t* emu, void* user_png_ver, void* error_ptr, void* error_fn, void* warn_fn, void* mem_ptr, void* malloc_fn, void* free_fn)
{
    library_t * lib = GetLibInternal(png12Name);
    png12_my_t *my = (png12_my_t*)lib->priv.w.p2;

    return my->png_create_read_struct_2(user_png_ver, error_ptr, finderrorFct(error_fn), findwarningFct(warn_fn), mem_ptr, findmallocFct(malloc_fn), findfreeFct(free_fn));
}

EXPORT void* my12_png_create_write_struct_2(x86emu_t* emu, void* user_png_ver, void* error_ptr, void* error_fn, void* warn_fn, void* mem_ptr, void* malloc_fn, void* free_fn)
{
    library_t * lib = GetLibInternal(png12Name);
    png12_my_t *my = (png12_my_t*)lib->priv.w.p2;

    return my->png_create_write_struct_2(user_png_ver, error_ptr, finderrorFct(error_fn), findwarningFct(warn_fn), mem_ptr, findmallocFct(malloc_fn), findfreeFct(free_fn));
}

EXPORT void my12_png_set_progressive_read_fn(x86emu_t* emu, void* png_ptr, void* user_ptr, void* info, void* row, void* end)
{
    library_t * lib = GetLibInternal(png12Name);
    png12_my_t *my = (png12_my_t*)lib->priv.w.p2;

    my->png_set_progressive_read_fn(png_ptr, user_ptr, findprogressive_infoFct(info), findprogressive_rowFct(row), findprogressive_endFct(end));
}

EXPORT void* my12_png_create_read_struct(x86emu_t* emu, void* png_ptr, void* user_ptr, void* errorfn, void* warnfn)
{
    library_t * lib = GetLibInternal(png12Name);
    png12_my_t *my = (png12_my_t*)lib->priv.w.p2;

    return my->png_create_read_struct(png_ptr, user_ptr, finderrorFct(errorfn), findwarningFct(warnfn));
}

// Maybe this is needed?
//#define CUSTOM_INIT     lib->priv.w.altprefix=strdup("yes");

#define CUSTOM_INIT \
    lib->priv.w.p2 = getPng12My(lib);   \
    lib->altmy = strdup("my12_");

#define CUSTOM_FINI \
    freePng12My(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"

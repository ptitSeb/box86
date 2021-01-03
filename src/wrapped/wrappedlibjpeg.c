#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <setjmp.h>

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

const char* libjpegName = "libjpeg.so.8";
#define LIBNAME libjpeg
#define ALTNAME "libjpeg.so.62"

static library_t* my_lib = NULL;
static bridge_t* my_bridge = NULL;

typedef void*   (*pFp_t)    (void*);
typedef int     (*iFp_t)    (void*);
typedef int     (*iFpi_t)   (void*, int);
typedef void    (*vFpip_t)  (void*, int, void*);
typedef void    (*vFpiL_t)  (void*, int, unsigned long);
typedef uint32_t(*uFppu_t)  (void*, void*, uint32_t);

#define SUPER() \
    GO(jpeg_CreateDecompress, vFpiL_t)  \
    GO(jpeg_read_header, iFpi_t)        \
    GO(jpeg_start_decompress, iFp_t)    \
    GO(jpeg_read_scanlines, uFppu_t)    \
    GO(jpeg_finish_decompress, iFp_t)   \
    GO(jpeg_std_error, pFp_t)           \
    GO(jpeg_set_marker_processor, vFpip_t)

typedef struct jpeg_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} jpeg_my_t;

void* getJpegMy(library_t* lib)
{
    jpeg_my_t* my = (jpeg_my_t*)calloc(1, sizeof(jpeg_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeJpegMy(void* lib)
{
    //jpeg_my_t *my = (jpeg_my_t *)lib;
}

typedef struct jpeg_error_mgr_s {
  void (*error_exit) (void* cinfo);
  void (*emit_message) (void* cinfo, int msg_level);
  void (*output_message) (void* cinfo);
  void (*format_message) (void* cinfo, char *buffer);
  void (*reset_error_mgr) (void* cinfo);
  int msg_code;
  union {
    int i[8];
    char s[80];
  } msg_parm;
  int trace_level;
  long num_warnings;
  const char * const *jpeg_message_table;
  int last_jpeg_message;
  const char * const *addon_message_table;
  int first_addon_message;
  int last_addon_message;
} jpeg_error_mgr_t;

typedef struct jpeg_common_struct_s {
  jpeg_error_mgr_t* err;
  void* mem;    //struct jpeg_memory_mgr
  void* progress;   //struct jpeg_progress_mgr
  void* client_data;
  int is_decompressor;
  int global_state;
} jpeg_common_struct_t;

static struct __jmp_buf_tag jmpbuf;
static int                  is_jmpbuf;

static void wrapErrorMgr(bridge_t* bridge, jpeg_error_mgr_t* mgr);
static void unwrapErrorMgr(bridge_t* bridge, jpeg_error_mgr_t* mgr);

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)

static x86emu_t* my_jpegcb_emu = NULL;\
// error_exit
#define GO(A)   \
static uintptr_t my_error_exit_fct_##A = 0;   \
static void my_error_exit_##A(jpeg_common_struct_t* cinfo)      \
{                                       \
    uintptr_t oldip = my_jpegcb_emu->ip.dword[0];               \
    wrapErrorMgr(my_bridge, cinfo->err);                        \
    RunFunctionWithEmu(my_jpegcb_emu, 1, my_error_exit_fct_##A, 1, cinfo);   \
    if(oldip==my_jpegcb_emu->ip.dword[0])                       \
        unwrapErrorMgr(my_bridge, cinfo->err);                  \
    else                                                        \
        if(is_jmpbuf) longjmp(&jmpbuf, 1);                     \
}
SUPER()
#undef GO
static void* finderror_exitFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_error_exit_fct_##A == (uintptr_t)fct) return my_error_exit_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_error_exit_fct_##A == 0) {my_error_exit_fct_##A = (uintptr_t)fct; return my_error_exit_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Jpeg error_exit callback\n");
    return NULL;
}
static void* is_error_exitFct(void* fct)
{
    if(!fct) return NULL;    
    #define GO(A) if(my_error_exit_##A == fct) return (void*)my_error_exit_fct_##A;
    SUPER()
    #undef GO
    return NULL;
}

// emit_message
#define GO(A)   \
static uintptr_t my_emit_message_fct_##A = 0;   \
static void my_emit_message_##A(void* cinfo, int msg_level)     \
{                                       \
    RunFunctionWithEmu(my_jpegcb_emu, 1, my_emit_message_fct_##A, 2, cinfo, msg_level);\
}
SUPER()
#undef GO
static void* findemit_messageFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_emit_message_fct_##A == (uintptr_t)fct) return my_emit_message_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_emit_message_fct_##A == 0) {my_emit_message_fct_##A = (uintptr_t)fct; return my_emit_message_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Jpeg emit_message callback\n");
    return NULL;
}
static void* is_emit_messageFct(void* fct)
{
    if(!fct) return NULL;
    #define GO(A) if(my_emit_message_##A == fct) return (void*)my_emit_message_fct_##A;
    SUPER()
    #undef GO
    return NULL;
}

// output_message
#define GO(A)   \
static uintptr_t my_output_message_fct_##A = 0;   \
static void my_output_message_##A(void* cinfo)     \
{                                       \
    RunFunctionWithEmu(my_jpegcb_emu, 1, my_output_message_fct_##A, 1, cinfo);\
}
SUPER()
#undef GO
static void* findoutput_messageFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_output_message_fct_##A == (uintptr_t)fct) return my_output_message_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_output_message_fct_##A == 0) {my_output_message_fct_##A = (uintptr_t)fct; return my_output_message_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Jpeg output_message callback\n");
    return NULL;
}
static void* is_output_messageFct(void* fct)
{
    if(!fct) return NULL;    
    #define GO(A) if(my_output_message_##A == fct) return (void*)my_output_message_fct_##A;
    SUPER()
    #undef GO
    return NULL;
}

// format_message
#define GO(A)   \
static uintptr_t my_format_message_fct_##A = 0;   \
static void my_format_message_##A(void* cinfo, char* buffer)     \
{                                       \
    RunFunctionWithEmu(my_jpegcb_emu, 1, my_format_message_fct_##A, 2, cinfo, buffer);\
}
SUPER()
#undef GO
static void* findformat_messageFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_format_message_fct_##A == (uintptr_t)fct) return my_format_message_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_format_message_fct_##A == 0) {my_format_message_fct_##A = (uintptr_t)fct; return my_format_message_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Jpeg format_message callback\n");
    return NULL;
}
static void* is_format_messageFct(void* fct)
{
    if(!fct) return NULL;    
    #define GO(A) if(my_format_message_##A == fct) return (void*)my_format_message_fct_##A;
    SUPER()
    #undef GO
    return NULL;
}

// reset_error_mgr
#define GO(A)   \
static uintptr_t my_reset_error_mgr_fct_##A = 0;   \
static void my_reset_error_mgr_##A(void* cinfo)     \
{                                       \
    RunFunctionWithEmu(my_jpegcb_emu, 1, my_reset_error_mgr_fct_##A, 1, cinfo);\
}
SUPER()
#undef GO
static void* findreset_error_mgrFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_reset_error_mgr_fct_##A == (uintptr_t)fct) return my_reset_error_mgr_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_reset_error_mgr_fct_##A == 0) {my_reset_error_mgr_fct_##A = (uintptr_t)fct; return my_reset_error_mgr_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Jpeg reset_error_mgr callback\n");
    return NULL;
}
static void* is_reset_error_mgrFct(void* fct)
{
    if(!fct) return NULL;    
    #define GO(A) if(my_reset_error_mgr_##A == fct) return (void*)my_reset_error_mgr_fct_##A;
    SUPER()
    #undef GO
    return NULL;
}

// jpeg_marker_parser_method
#define GO(A)   \
static uintptr_t my_jpeg_marker_parser_method_fct_##A = 0;   \
static int my_jpeg_marker_parser_method_##A(void* cinfo)    \
{                                       \
    return (int)RunFunction(my_context, my_jpeg_marker_parser_method_fct_##A, 1, cinfo);\
}
SUPER()
#undef GO
static void* findjpeg_marker_parser_methodFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_jpeg_marker_parser_method_fct_##A == (uintptr_t)fct) return my_jpeg_marker_parser_method_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_jpeg_marker_parser_method_fct_##A == 0) {my_jpeg_marker_parser_method_fct_##A = (uintptr_t)fct; return my_jpeg_marker_parser_method_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libpng12 jpeg_marker_parser_method callback\n");
    return NULL;
}

#undef SUPER

#define SUPER() \
    GO(error_exit, vFp)      \
    GO(emit_message, vFpi)   \
    GO(output_message, vFp)  \
    GO(format_message, vFpp) \
    GO(reset_error_mgr, vFp)

static void wrapErrorMgr(bridge_t* bridge, jpeg_error_mgr_t* mgr)
{
    if(!mgr)
        return;
    void *p;
    #define GO(A, W)    \
        p = is_##A##Fct(mgr->A);                                \
        if(!p) p = (void*)AddCheckBridge(bridge, W, mgr->A, 0); \
        mgr->A = p;

    SUPER()
    #undef GO
}

static void unwrapErrorMgr(bridge_t* bridge, jpeg_error_mgr_t* mgr)
{
    if(!mgr)
        return;
    void* p;
    #define GO(A, W)    \
        p = GetNativeFnc((uintptr_t)mgr->A);    \
        if(!p)                                  \
            p = find##A##Fct(mgr->A);   \
        mgr->A = p;
        
    SUPER()
    #undef GO
}


EXPORT int my_jpeg_simd_cpu_support()
{
    return 0x01|0x04|0x08; // MMX/SSE/SSE2
}

EXPORT void* my_jpeg_std_error(x86emu_t* emu, void* errmgr)
{
    jpeg_my_t *my = (jpeg_my_t*)my_lib->priv.w.p2;

    jpeg_error_mgr_t* ret = my->jpeg_std_error(errmgr);

    wrapErrorMgr(my_lib->priv.w.bridge, ret);
trace_end = 0;

    return ret;
}

#define WRAP(T, A)          \
    jpeg_my_t *my = (jpeg_my_t*)my_lib->priv.w.p2;      \
    is_jmpbuf = 1;          \
    my_jpegcb_emu = emu;    \
    unwrapErrorMgr(my_lib->priv.w.bridge, cinfo->err);  \
    if(setjmp(&jmpbuf)) {   \
        wrapErrorMgr(my_lib->priv.w.bridge, cinfo->err);\
        is_jmpbuf = 0;      \
        return (T)R_EAX;    \
    }                       \
    A;                      \
    is_jmpbuf = 0;          \
    wrapErrorMgr(my_lib->priv.w.bridge, cinfo->err)


EXPORT void my_jpeg_CreateDecompress(x86emu_t* emu, jpeg_common_struct_t* cinfo, int version, unsigned long structsize)
{
    WRAP(void, my->jpeg_CreateDecompress(cinfo, version, structsize));
}

EXPORT int my_jpeg_read_header(x86emu_t* emu, jpeg_common_struct_t* cinfo, int image)
{
    WRAP(int, int ret = my->jpeg_read_header(cinfo, image));
    return ret;
}

EXPORT int my_jpeg_start_decompress(x86emu_t* emu, jpeg_common_struct_t* cinfo)
{
    WRAP(int, int ret = my->jpeg_start_decompress(cinfo));
    return ret;
}

EXPORT uint32_t my_jpeg_read_scanlines(x86emu_t* emu, jpeg_common_struct_t* cinfo, void* scanlines, uint32_t maxlines)
{
    WRAP(uint32_t, uint32_t ret = my->jpeg_read_scanlines(cinfo, scanlines, maxlines));
    return ret;
}

EXPORT int my_jpeg_finish_decompress(x86emu_t* emu, jpeg_common_struct_t* cinfo)
{
    WRAP(int, int ret = my->jpeg_finish_decompress(cinfo));
    return ret;
}

EXPORT void my_jpeg_set_marker_processor(x86emu_t* emu, jpeg_common_struct_t* cinfo, int marker, void* routine)
{
    WRAP(void, my->jpeg_set_marker_processor(cinfo, marker, findjpeg_marker_parser_methodFct(routine)));
}

#undef WRAP

#define CUSTOM_INIT \
    my_bridge = lib->priv.w.bridge;     \
    my_lib = lib;                       \
    lib->priv.w.p2 = getJpegMy(lib);

#define CUSTOM_FINI \
    freeJpegMy(lib->priv.w.p2); \
    free(lib->priv.w.p2);       \
    my_lib = NULL;

#include "wrappedlib_init.h"

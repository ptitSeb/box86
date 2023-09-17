#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <setjmp.h>
#include <stddef.h>

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

const char* libjpeg62Name = "libjpeg.so.62";
#define LIBNAME libjpeg62
#define ALTNAME "libjpeg.so.8"

static bridge_t* my_bridge = NULL;

#define ADDED_FUNCTIONS()           \

#include "generated/wrappedlibjpeg62types.h"

#include "wrappercallback.h"

typedef struct jpeg62_error_mgr_s {
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
} jpeg62_error_mgr_t;

typedef struct jpeg62_memory_mgr_s {
  void* (*alloc_small) (void* cinfo, int pool_id, size_t sizeofobject);
  void* (*alloc_large) (void* cinfo, int pool_id, size_t sizeofobject);
  void* (*alloc_sarray) (void* cinfo, int pool_id, uint32_t samplesperrow, uint32_t numrows);
  void* (*alloc_barray) (void* cinfo, int pool_id, uint32_t blocksperrow, uint32_t numrows);
  void* (*request_virt_sarray) (void* cinfo, int pool_id,
                                           int pre_zero,
                                           uint32_t samplesperrow,
                                           uint32_t numrows,
                                           uint32_t maxaccess);
  void* (*request_virt_barray) (void* cinfo, int pool_id,
                                           int pre_zero,
                                           uint32_t blocksperrow,
                                           uint32_t numrows,
                                           uint32_t maxaccess);
  void (*realize_virt_arrays) (void* cinfo);
  void* (*access_virt_sarray) (void* cinfo, void* ptr,
                                    uint32_t start_row, uint32_t num_rows,
                                    int writable);
  void* (*access_virt_barray) (void* cinfo, void* ptr,
                                     uint32_t start_row, uint32_t num_rows,
                                     int writable);
  void (*free_pool) (void* cinfo, int pool_id);
  void (*self_destruct) (void* cinfo);
  long max_memory_to_use;
  long max_alloc_chunk;
} jpeg62_memory_mgr_t;


typedef struct jpeg62_common_struct_s {
  jpeg62_error_mgr_t* err;
  jpeg62_memory_mgr_t* mem;
  void* progress;   //struct jpeg_progress_mgr
  void* client_data;
  int is_decompressor;
  int global_state;
} jpeg62_common_struct_t;

typedef struct jpeg62_source_mgr_t {
  void *next_input_byte;
  size_t bytes_in_buffer;
  void (*init_source) (void* cinfo);
  int (*fill_input_buffer) (void* cinfo);
  void (*skip_input_data) (void* cinfo, long num_bytes);
  int (*resync_to_restart) (void* cinfo, int desired);
  void (*term_source) (void* cinfo);
} jpeg62_source_mgr_t;

// simplied structure
typedef struct j62_decompress_ptr_s
{
  jpeg62_error_mgr_t* err;
  jpeg62_memory_mgr_t* mem;
  void* progress;   //struct jpeg_progress_mgr
  void* client_data;
  int is_decompressor;
  int global_state;

  jpeg62_source_mgr_t *src;
  // other stuff not needed for wraping
} j62_decompress_ptr_t;

typedef struct jpeg62_destination_mgr_s {
  void *next_output_byte;
  size_t free_in_buffer;

  void (*init_destination) (void* cinfo);
  int (*empty_output_buffer) (void* cinfo);
  void (*term_destination) (void* cinfo);
} jpeg62_destination_mgr_t;

#ifdef ANDROID
#define JUMPBUFF sigjmp_buf
#else
#define JUMPBUFF struct __jmp_buf_tag
#endif

typedef struct jmpbuf_helper {
    JUMPBUFF jmpbuf;
    void* client_data;
    void* compress;
}jmpbuf_helper;

#define RunFunctionWithEmu_helper(...) \
    jmpbuf_helper* helper = (jmpbuf_helper*)((jpeg62_common_struct_t*)cinfo)->client_data; \
    ((jpeg62_common_struct_t*)cinfo)->client_data = helper->client_data; \
    uint32_t ret = RunFunctionWithEmu(__VA_ARGS__); \
    ((jpeg62_common_struct_t*)cinfo)->client_data = helper;

#define RunFunction_helper(...) \
    jmpbuf_helper* helper = ((jpeg62_common_struct_t*)cinfo)->client_data; \
    ((jpeg62_common_struct_t*)cinfo)->client_data = helper->client_data; \
    uint32_t ret = RunFunctionFmt(__VA_ARGS__); \
    ((jpeg62_common_struct_t*)cinfo)->client_data = helper;

#define RunFunction_helper2(...) \
    jmpbuf_helper* helper = ((jpeg62_common_struct_t*)cinfo)->client_data; \
    tmp->client_data = helper->client_data;     \
    uint32_t ret = RunFunctionFmt(__VA_ARGS__);    \
    tmp->client_data = helper;

#define COMPRESS_STRUCT                 \
  GOM(jpeg62_error_mgr_t*, err)         \
  GOM(jpeg62_memory_mgr_t*, mem)        \
  GO(void*, progress)                   \
  GO(void*, client_data)                \
  GO(int, is_decompressor)              \
  GO(int, global_state)                 \
                                        \
  GOM(jpeg62_destination_mgr_t*, dest)  \
  GO(uint32_t, image_width)             \
  GO(uint32_t, image_height)            \
  GO(int, input_components)             \
  GO(int, in_color_space)               \
  GO2(double, input_gamma, uint32_t, 2) \
                                        \
  GO(int, data_precision)               \
  GO(int, num_components)               \
  GO(int, jpeg_color_space)             \
  GO(void*, comp_info)                  \
  GOA(void*, quant_tbl_ptrs, 4);        \
                                        \
  GOA(void*, dc_huff_tbl_ptrs, 4)       \
  GOA(void*, ac_huff_tbl_ptrs, 4)       \
  GOA(uint8_t, arith_dc_L, 16)          \
  GOA(uint8_t, arith_dc_U, 16)          \
  GOA(uint8_t, arith_ac_K, 16)          \
  GO(int, num_scans)                    \
  GO(void*, scan_info)                  \
  GO(int, raw_data_in)                  \
  GO(int, arith_code)                   \
  GO(int, optimize_coding)              \
  GO(int, CCIR601_sampling)             \
                                        \
  GO(int, smoothing_factor)             \
  GO(int, dct_method)                   \
  GO(uint32_t, restart_interval)        \
  GO(int, restart_in_rows)              \
  GO(int, write_JFIF_header)            \
  GO(uint8_t, JFIF_major_version)       \
  GO(uint8_t, JFIF_minor_version)       \
  GO(uint8_t, density_unit)             \
  GO(uint16_t, X_density)               \
  GO(uint16_t, Y_density)               \
  GO(int, write_Adobe_marker)           \
  GO(uint32_t, next_scanline)           \
  GO(int, progressive_mode)             \
  GO(int, max_h_samp_factor)            \
  GO(int, max_v_samp_factor)            \
                                        \
  GO(uint32_t, total_iMCU_rows)         \
  GO(int, comps_in_scan)                \
  GOA(void*, cur_comp_info, 4)          \
  GO(uint32_t,  MCUs_per_row)           \
  GO(uint32_t,  MCU_rows_in_scan)       \
  GO(int,  blocks_in_MCU)               \
  GOA(int, MCU_membership, 10)          \
  GO(int, Ss)                           \
  GO(int, Se)                           \
  GO(int, Ah)                           \
  GO(int, Al)                           \
                                        \
  GO(void*, master)                     \
  GO(void*, main)                       \
  GO(void*, prep)                       \
  GO(void*, coef)                       \
  GO(void*, marker)                     \
  GO(void*, cconvert)                   \
  GO(void*, downsample)                 \
  GO(void*, fdct)                       \
  GO(void*, entropy)                    \
  GO(void*, script_space)               \
  GO(int, script_space_size)            \

#define GO(A, B)        A B;
#define GO2(A, B, C, D) A B;
#define GOM(A, B)       A B;
#define GOA(A, B, C)    A B[C];
typedef struct j62_compress_ptr_s
{
    COMPRESS_STRUCT
} j62_compress_ptr_t;
#undef GO2
#define GO2(A, B, C, D) C B[D];
typedef struct i386_compress_ptr_s
{
    COMPRESS_STRUCT
} i386_compress_ptr_t;
#undef GOA
#undef GOM
#undef GO2
#undef GO
#undef COMPRESS_STRUCT

typedef struct jpeg62_comp_master_s {
  void (*prepare_for_pass) (j62_compress_ptr_t* cinfo);
  void (*pass_startup) (j62_compress_ptr_t* cinfo);
  void (*finish_pass) (j62_compress_ptr_t* cinfo);

  /* State variables made visible to other modules */
  int call_pass_startup;    /* True if pass_startup must be called */
  int is_last_pass;         /* True during last pass */
}jpeg62_comp_master_t;

void j62compress_i386_to_arm(j62_compress_ptr_t* d, i386_compress_ptr_t* s) {
    memcpy(d, s, offsetof(struct i386_compress_ptr_s, input_gamma));
    memcpy(&d->input_gamma, &s->input_gamma, sizeof(struct i386_compress_ptr_s) - offsetof(struct i386_compress_ptr_s, input_gamma));
}

void j62compress_arm_to_i386(i386_compress_ptr_t* d, j62_compress_ptr_t* s) {
    memcpy(d, s, offsetof(struct i386_compress_ptr_s, input_gamma));
    memcpy(&d->input_gamma, &s->input_gamma, sizeof(struct i386_compress_ptr_s) - offsetof(struct i386_compress_ptr_s, input_gamma));
}

static jpeg62_error_mgr_t native_err_mgr;

#define SUPER()         \
    jpeg62_common_struct_t temp_cinfo = *cinfo; \
    jpeg62_error_mgr_t err = *cinfo->err; \
    temp_cinfo.err = &err; \
    GO(error_exit)      \
    GO(emit_message)    \
    GO(output_message)  \
    GO(format_message)  \
    GO(reset_error_mgr)

#define GO(A) \
        temp_cinfo.err->A = GetNativeFncOrFnc((uintptr_t)cinfo->err->A); 

static void native_error_exit(jpeg62_common_struct_t* cinfo) {
    SUPER();
    native_err_mgr.error_exit(&temp_cinfo);
}

static void native_emit_message(jpeg62_common_struct_t* cinfo, int msg_level) {
    SUPER();
    native_err_mgr.emit_message(&temp_cinfo, msg_level);
}

static void native_output_message(jpeg62_common_struct_t* cinfo) {
    SUPER();
    native_err_mgr.output_message(&temp_cinfo);
}

static void native_format_message(jpeg62_common_struct_t* cinfo, char* buffer) {
    SUPER();
    native_err_mgr.format_message(&temp_cinfo, buffer);
}

static void native_reset_error_mgr(jpeg62_common_struct_t* cinfo) {
    SUPER();
    native_err_mgr.reset_error_mgr(&temp_cinfo);
}

#undef GO
#undef SUPER

static void wrapErrorMgr(jpeg62_error_mgr_t* mgr);
static void unwrapErrorMgr(jpeg62_error_mgr_t* mgr);
static void wrapMemoryMgr(jpeg62_memory_mgr_t* mgr);
static void unwrapMemoryMgr(jpeg62_memory_mgr_t* mgr);
static void wrapCompMaster(jpeg62_comp_master_t* master);
static void unwrapCompMaster(jpeg62_comp_master_t* master);

static void wrapCommonStruct(void* s, int type);
static void unwrapCommonStruct(void* s, int type);
static void wrapCompressStruct(i386_compress_ptr_t* d, j62_compress_ptr_t* s);
static void unwrapCompressStruct(j62_compress_ptr_t* d, i386_compress_ptr_t* s);

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)

// error_exit
#define GO(A)   \
static uintptr_t my_error_exit_fct_##A = 0;                         \
static void my_error_exit_##A(jpeg62_common_struct_t* cinfo)        \
{                                                                   \
    uintptr_t oldip = thread_get_emu()->ip.dword[0];                \
    wrapCommonStruct(cinfo, 0);                                     \
    RunFunctionWithEmu_helper(thread_get_emu(), 1, my_error_exit_fct_##A, 1, cinfo);   \
    if(oldip==thread_get_emu()->ip.dword[0])                        \
        unwrapCommonStruct(cinfo, 0);                               \
    else                                                            \
        longjmp(cinfo->client_data, 1);                             \
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
static void* reverse_error_exitFct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_error_exit_##A == fct) return (void*)my_error_exit_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, vFp, fct, 0, NULL);
}

// emit_message
#define GO(A)   \
static uintptr_t my_emit_message_fct_##A = 0;   \
static void my_emit_message_##A(void* cinfo, int msg_level)     \
{                                       \
    RunFunctionWithEmu_helper(thread_get_emu(), 1, my_emit_message_fct_##A, 2, cinfo, msg_level);\
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
static void* reverse_emit_messageFct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_emit_message_##A == fct) return (void*)my_emit_message_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, vFpi, fct, 0, NULL);
}

// output_message
#define GO(A)   \
static uintptr_t my_output_message_fct_##A = 0;   \
static void my_output_message_##A(void* cinfo)     \
{                                       \
    RunFunctionWithEmu_helper(thread_get_emu(), 1, my_output_message_fct_##A, 1, cinfo);\
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
static void* reverse_output_messageFct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_output_message_##A == fct) return (void*)my_output_message_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, vFp, fct, 0, NULL);
}

// format_message
#define GO(A)   \
static uintptr_t my_format_message_fct_##A = 0;   \
static void my_format_message_##A(void* cinfo, char* buffer)     \
{                                       \
    RunFunctionWithEmu_helper(thread_get_emu(), 1, my_format_message_fct_##A, 2, cinfo, buffer);\
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
static void* reverse_format_messageFct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_format_message_##A == fct) return (void*)my_format_message_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, vFpp, fct, 0, NULL);
}

// reset_error_mgr
#define GO(A)   \
static uintptr_t my_reset_error_mgr_fct_##A = 0;   \
static void my_reset_error_mgr_##A(void* cinfo)     \
{                                       \
    RunFunctionWithEmu_helper(thread_get_emu(), 1, my_reset_error_mgr_fct_##A, 1, cinfo);\
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
static void* reverse_reset_error_mgrFct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_reset_error_mgr_##A == fct) return (void*)my_reset_error_mgr_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, vFp, fct, 0, NULL);
}

// jpeg_marker_parser_method
#define GO(A)   \
static uintptr_t my_jpeg_marker_parser_method_fct_##A = 0;   \
static int my_jpeg_marker_parser_method_##A(void* cinfo)    \
{                                       \
    RunFunction_helper(my_jpeg_marker_parser_method_fct_##A, "p", cinfo);\
    return (int)ret; \
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
    printf_log(LOG_NONE, "Warning, no more slot for libjpeg.so.62 jpeg_marker_parser_method callback\n");
    return NULL;
}
/* static void* reverse_jpeg_marker_parser_methodFct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_jpeg_marker_parser_method_##A == fct) return (void*)my_jpeg_marker_parser_method_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, iFp, fct, 0, NULL);
} */

// alloc_small
#define GO(A)   \
static uintptr_t my_alloc_small_fct_##A = 0;   \
static void* my_alloc_small_##A(void* cinfo, int pool_id, size_t sizeofobject)    \
{                                       \
    RunFunction_helper(my_alloc_small_fct_##A, "piL", cinfo, pool_id, sizeofobject);\
    return (void*)ret; \
}
SUPER()
#undef GO
static void* findalloc_smallFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_alloc_small_fct_##A == (uintptr_t)fct) return my_alloc_small_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_alloc_small_fct_##A == 0) {my_alloc_small_fct_##A = (uintptr_t)fct; return my_alloc_small_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libjpeg.so.62 alloc_small callback\n");
    return NULL;
}
static void* reverse_alloc_smallFct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_alloc_small_##A == fct) return (void*)my_alloc_small_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, pFpiL, fct, 0, NULL);
}

// alloc_large
#define GO(A)   \
static uintptr_t my_alloc_large_fct_##A = 0;   \
static void* my_alloc_large_##A(void* cinfo, int pool_id, size_t sizeofobject)    \
{                                       \
    RunFunction_helper(my_alloc_large_fct_##A, "piL", cinfo, pool_id, sizeofobject);\
    return (void*)ret; \
}
SUPER()
#undef GO
static void* findalloc_largeFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_alloc_large_fct_##A == (uintptr_t)fct) return my_alloc_large_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_alloc_large_fct_##A == 0) {my_alloc_large_fct_##A = (uintptr_t)fct; return my_alloc_large_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libjpeg.so.62 alloc_large callback\n");
    return NULL;
}
static void* reverse_alloc_largeFct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_alloc_large_##A == fct) return (void*)my_alloc_large_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, pFpiL, fct, 0, NULL);
}

// alloc_sarray
#define GO(A)   \
static uintptr_t my_alloc_sarray_fct_##A = 0;   \
static void* my_alloc_sarray_##A(void* cinfo, int pool_id, uint32_t samplesperrow, uint32_t numrows)    \
{                                       \
    RunFunction_helper(my_alloc_sarray_fct_##A, "piuu", cinfo, pool_id, samplesperrow, numrows);\
    return (void*)ret; \
}
SUPER()
#undef GO
static void* findalloc_sarrayFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_alloc_sarray_fct_##A == (uintptr_t)fct) return my_alloc_sarray_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_alloc_sarray_fct_##A == 0) {my_alloc_sarray_fct_##A = (uintptr_t)fct; return my_alloc_sarray_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libjpeg.so.62 alloc_sarray callback\n");
    return NULL;
}
static void* reverse_alloc_sarrayFct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_alloc_sarray_##A == fct) return (void*)my_alloc_sarray_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, pFpiuu, fct, 0, NULL);
}

// alloc_barray
#define GO(A)   \
static uintptr_t my_alloc_barray_fct_##A = 0;   \
static void* my_alloc_barray_##A(void* cinfo, int pool_id, uint32_t samplesperrow, uint32_t numrows)    \
{                                       \
    RunFunction_helper(my_alloc_barray_fct_##A, "piuu", cinfo, pool_id, samplesperrow, numrows);\
    return (void*)ret; \
}
SUPER()
#undef GO
static void* findalloc_barrayFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_alloc_barray_fct_##A == (uintptr_t)fct) return my_alloc_barray_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_alloc_barray_fct_##A == 0) {my_alloc_barray_fct_##A = (uintptr_t)fct; return my_alloc_barray_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libjpeg.so.62 alloc_barray callback\n");
    return NULL;
}
static void* reverse_alloc_barrayFct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_alloc_barray_##A == fct) return (void*)my_alloc_barray_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, pFpiuu, fct, 0, NULL);
}

// request_virt_sarray
#define GO(A)   \
static uintptr_t my_request_virt_sarray_fct_##A = 0;   \
static void* my_request_virt_sarray_##A(void* cinfo, int pool_id, int pre_zero, uint32_t samplesperrow,uint32_t numrows, uint32_t maxaccess)    \
{                                       \
    RunFunction_helper(my_request_virt_sarray_fct_##A, "piiuuu", cinfo, pool_id, pre_zero, samplesperrow, numrows, maxaccess);\
    return (void*)ret; \
}
SUPER()
#undef GO
static void* findrequest_virt_sarrayFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_request_virt_sarray_fct_##A == (uintptr_t)fct) return my_request_virt_sarray_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_request_virt_sarray_fct_##A == 0) {my_request_virt_sarray_fct_##A = (uintptr_t)fct; return my_request_virt_sarray_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libjpeg.so.62 request_virt_sarray callback\n");
    return NULL;
}
static void* reverse_request_virt_sarrayFct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_request_virt_sarray_##A == fct) return (void*)my_request_virt_sarray_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, pFpiiuuu, fct, 0, NULL);
}

// request_virt_barray
#define GO(A)   \
static uintptr_t my_request_virt_barray_fct_##A = 0;   \
static void* my_request_virt_barray_##A(void* cinfo, int pool_id, int pre_zero, uint32_t samplesperrow,uint32_t numrows, uint32_t maxaccess)    \
{                                       \
    RunFunction_helper(my_request_virt_barray_fct_##A, "piiuuu", cinfo, pool_id, pre_zero, samplesperrow, numrows, maxaccess);\
    return (void*)ret; \
}
SUPER()
#undef GO
static void* findrequest_virt_barrayFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_request_virt_barray_fct_##A == (uintptr_t)fct) return my_request_virt_barray_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_request_virt_barray_fct_##A == 0) {my_request_virt_barray_fct_##A = (uintptr_t)fct; return my_request_virt_barray_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libjpeg.so.62 request_virt_barray callback\n");
    return NULL;
}
static void* reverse_request_virt_barrayFct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_request_virt_barray_##A == fct) return (void*)my_request_virt_barray_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, pFpiiuuu, fct, 0, NULL);
}

// realize_virt_arrays
#define GO(A)   \
static uintptr_t my_realize_virt_arrays_fct_##A = 0;   \
static void my_realize_virt_arrays_##A(void* cinfo)    \
{                                       \
    RunFunction_helper(my_realize_virt_arrays_fct_##A, "p", cinfo);\
}
SUPER()
#undef GO
static void* findrealize_virt_arraysFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_realize_virt_arrays_fct_##A == (uintptr_t)fct) return my_realize_virt_arrays_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_realize_virt_arrays_fct_##A == 0) {my_realize_virt_arrays_fct_##A = (uintptr_t)fct; return my_realize_virt_arrays_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libjpeg.so.62 realize_virt_arrays callback\n");
    return NULL;
}
static void* reverse_realize_virt_arraysFct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_realize_virt_arrays_##A == fct) return (void*)my_realize_virt_arrays_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, vFp, fct, 0, NULL);
}

// access_virt_sarray
#define GO(A)   \
static uintptr_t my_access_virt_sarray_fct_##A = 0;   \
static void* my_access_virt_sarray_##A(void* cinfo, void* ptr, uint32_t start_row, uint32_t num_rows, int writable)    \
{                                       \
    RunFunction_helper(my_access_virt_sarray_fct_##A, "ppuui", cinfo, ptr, start_row, num_rows, writable);\
    return (void*)ret; \
}
SUPER()
#undef GO
static void* findaccess_virt_sarrayFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_access_virt_sarray_fct_##A == (uintptr_t)fct) return my_access_virt_sarray_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_access_virt_sarray_fct_##A == 0) {my_access_virt_sarray_fct_##A = (uintptr_t)fct; return my_access_virt_sarray_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libjpeg.so.62 access_virt_sarray callback\n");
    return NULL;
}
static void* reverse_access_virt_sarrayFct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_access_virt_sarray_##A == fct) return (void*)my_access_virt_sarray_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, pFppuui, fct, 0, NULL);
}

// access_virt_barray
#define GO(A)   \
static uintptr_t my_access_virt_barray_fct_##A = 0;   \
static void* my_access_virt_barray_##A(void* cinfo, void* ptr, uint32_t start_row, uint32_t num_rows, int writable)    \
{                                       \
    RunFunction_helper(my_access_virt_barray_fct_##A, "ppuui", cinfo, ptr, start_row, num_rows, writable);\
    return (void*)ret; \
}
SUPER()
#undef GO
static void* findaccess_virt_barrayFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_access_virt_barray_fct_##A == (uintptr_t)fct) return my_access_virt_barray_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_access_virt_barray_fct_##A == 0) {my_access_virt_barray_fct_##A = (uintptr_t)fct; return my_access_virt_barray_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libjpeg.so.62 access_virt_barray callback\n");
    return NULL;
}
static void* reverse_access_virt_barrayFct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_access_virt_barray_##A == fct) return (void*)my_access_virt_barray_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, pFppuui, fct, 0, NULL);
}

// free_pool
#define GO(A)   \
static uintptr_t my_free_pool_fct_##A = 0;   \
static void my_free_pool_##A(void* cinfo, int pool_id)    \
{                                       \
    RunFunction_helper(my_free_pool_fct_##A, "pi", cinfo, pool_id);\
}
SUPER()
#undef GO
static void* findfree_poolFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_free_pool_fct_##A == (uintptr_t)fct) return my_free_pool_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_free_pool_fct_##A == 0) {my_free_pool_fct_##A = (uintptr_t)fct; return my_free_pool_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libjpeg.so.62 free_pool callback\n");
    return NULL;
}
static void* reverse_free_poolFct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_free_pool_##A == fct) return (void*)my_free_pool_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, vFpi, fct, 0, NULL);
}

// self_destruct
#define GO(A)   \
static uintptr_t my_self_destruct_fct_##A = 0;   \
static void my_self_destruct_##A(void* cinfo)    \
{                                       \
    RunFunction_helper(my_self_destruct_fct_##A, "p", cinfo);\
}
SUPER()
#undef GO
static void* findself_destructFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_self_destruct_fct_##A == (uintptr_t)fct) return my_self_destruct_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_self_destruct_fct_##A == 0) {my_self_destruct_fct_##A = (uintptr_t)fct; return my_self_destruct_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libjpeg.so.62 self_destruct callback\n");
    return NULL;
}
static void* reverse_self_destructFct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_self_destruct_##A == fct) return (void*)my_self_destruct_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, vFp, fct, 0, NULL);
}

// init_source
#define GO(A)   \
static uintptr_t my_init_source_fct_##A = 0;   \
static void my_init_source_##A(void* cinfo)    \
{                                       \
    RunFunction_helper(my_init_source_fct_##A, "p", cinfo);\
}
SUPER()
#undef GO
static void* findinit_sourceFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_init_source_fct_##A == (uintptr_t)fct) return my_init_source_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_init_source_fct_##A == 0) {my_init_source_fct_##A = (uintptr_t)fct; return my_init_source_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libjpeg.so.62 init_source callback\n");
    return NULL;
}
static void* reverse_init_sourceFct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_init_source_##A == fct) return (void*)my_init_source_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, vFp, fct, 0, NULL);
}

// fill_input_buffer
#define GO(A)   \
static uintptr_t my_fill_input_buffer_fct_##A = 0;   \
static int my_fill_input_buffer_##A(void* cinfo)    \
{                                       \
    RunFunction_helper(my_fill_input_buffer_fct_##A, "p", cinfo);\
    return (int)ret; \
}
SUPER()
#undef GO
static void* findfill_input_bufferFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_fill_input_buffer_fct_##A == (uintptr_t)fct) return my_fill_input_buffer_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_fill_input_buffer_fct_##A == 0) {my_fill_input_buffer_fct_##A = (uintptr_t)fct; return my_fill_input_buffer_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libjpeg.so.62 fill_input_buffer callback\n");
    return NULL;
}
static void* reverse_fill_input_bufferFct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_fill_input_buffer_##A == fct) return (void*)my_fill_input_buffer_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, iFp, fct, 0, NULL);
}

// skip_input_data
#define GO(A)   \
static uintptr_t my_skip_input_data_fct_##A = 0;   \
static void my_skip_input_data_##A(void* cinfo, long num_bytes)    \
{                                       \
    RunFunction_helper(my_skip_input_data_fct_##A, "pl", cinfo, num_bytes);\
}
SUPER()
#undef GO
static void* findskip_input_dataFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_skip_input_data_fct_##A == (uintptr_t)fct) return my_skip_input_data_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_skip_input_data_fct_##A == 0) {my_skip_input_data_fct_##A = (uintptr_t)fct; return my_skip_input_data_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libjpeg.so.62 skip_input_data callback\n");
    return NULL;
}
static void* reverse_skip_input_dataFct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_skip_input_data_##A == fct) return (void*)my_skip_input_data_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, vFpl, fct, 0, NULL);
}

// resync_to_restart
#define GO(A)   \
static uintptr_t my_resync_to_restart_fct_##A = 0;   \
static int my_resync_to_restart_##A(void* cinfo, int desired)    \
{                                       \
    RunFunction_helper(my_resync_to_restart_fct_##A, "pi", cinfo, desired);\
    return (int)ret; \
}
SUPER()
#undef GO
static void* findresync_to_restartFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_resync_to_restart_fct_##A == (uintptr_t)fct) return my_resync_to_restart_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_resync_to_restart_fct_##A == 0) {my_resync_to_restart_fct_##A = (uintptr_t)fct; return my_resync_to_restart_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libjpeg.so.62 resync_to_restart callback\n");
    return NULL;
}
static void* reverse_resync_to_restartFct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_resync_to_restart_##A == fct) return (void*)my_resync_to_restart_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, iFpi, fct, 0, NULL);
}

// term_source
#define GO(A)   \
static uintptr_t my_term_source_fct_##A = 0;   \
static void my_term_source_##A(void* cinfo)    \
{                                       \
    RunFunction_helper(my_term_source_fct_##A, "p", cinfo);\
}
SUPER()
#undef GO
static void* findterm_sourceFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_term_source_fct_##A == (uintptr_t)fct) return my_term_source_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_term_source_fct_##A == 0) {my_term_source_fct_##A = (uintptr_t)fct; return my_term_source_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libjpeg.so.62 term_source callback\n");
    return NULL;
}
static void* reverse_term_sourceFct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_term_source_##A == fct) return (void*)my_term_source_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, vFp, fct, 0, NULL);
}

// init_destination
#define GO(A)   \
static uintptr_t my_init_destination_fct_##A = 0;                           \
static void my_init_destination_##A(j62_compress_ptr_t* cinfo)              \
{                                                                           \
    i386_compress_ptr_t* tmp = ((jmpbuf_helper*)cinfo->client_data)->compress;  \
    wrapCompressStruct(tmp, cinfo);                                         \
    RunFunction_helper2(my_init_destination_fct_##A, "p", tmp);             \
    unwrapCompressStruct(cinfo, tmp);                                       \
    tmp->client_data = ((jmpbuf_helper*)cinfo->client_data)->client_data;   \
}
SUPER()
#undef GO
static void* findinit_destinationFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_init_destination_fct_##A == (uintptr_t)fct) return my_init_destination_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_init_destination_fct_##A == 0) {my_init_destination_fct_##A = (uintptr_t)fct; return my_init_destination_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libjpeg.so.62 init_destination callback\n");
    return NULL;
}
static void* reverse_init_destinationFct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_init_destination_##A == fct) return (void*)my_init_destination_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, vFp, fct, 0, NULL);
}
// empty_output_buffer
#define GO(A)   \
static uintptr_t my_empty_output_buffer_fct_##A = 0;   \
static void my_empty_output_buffer_##A(j62_compress_ptr_t* cinfo)               \
{                                                                               \
    i386_compress_ptr_t* tmp = ((jmpbuf_helper*)cinfo->client_data)->compress;  \
    wrapCompressStruct(tmp, cinfo);                                             \
    RunFunction_helper2(my_empty_output_buffer_fct_##A, "p", tmp);              \
    unwrapCompressStruct(cinfo, tmp);                                           \
    tmp->client_data = ((jmpbuf_helper*)cinfo->client_data)->client_data;       \
}
SUPER()
#undef GO
static void* findempty_output_bufferFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_empty_output_buffer_fct_##A == (uintptr_t)fct) return my_empty_output_buffer_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_empty_output_buffer_fct_##A == 0) {my_empty_output_buffer_fct_##A = (uintptr_t)fct; return my_empty_output_buffer_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libjpeg.so.62 empty_output_buffer callback\n");
    return NULL;
}
static void* reverse_empty_output_bufferFct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_empty_output_buffer_##A == fct) return (void*)my_empty_output_buffer_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, vFp, fct, 0, NULL);
}
// term_destination
#define GO(A)   \
static uintptr_t my_term_destination_fct_##A = 0;   \
static void my_term_destination_##A(j62_compress_ptr_t* cinfo)              \
{                                                                           \
    i386_compress_ptr_t* tmp = ((jmpbuf_helper*)cinfo->client_data)->compress;  \
    wrapCompressStruct(tmp, cinfo);                                         \
    RunFunction_helper2(my_term_destination_fct_##A, "p", tmp);             \
    unwrapCompressStruct(cinfo, tmp);                                       \
    tmp->client_data = ((jmpbuf_helper*)cinfo->client_data)->client_data;   \
}
SUPER()
#undef GO
static void* findterm_destinationFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_term_destination_fct_##A == (uintptr_t)fct) return my_term_destination_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_term_destination_fct_##A == 0) {my_term_destination_fct_##A = (uintptr_t)fct; return my_term_destination_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libjpeg.so.62 term_destination callback\n");
    return NULL;
}
static void* reverse_term_destinationFct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_term_destination_##A == fct) return (void*)my_term_destination_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, vFp, fct, 0, NULL);
}

// prepare_for_pass
#define GO(A)   \
static uintptr_t my_prepare_for_pass_fct_##A = 0;   \
static void my_prepare_for_pass_##A(j62_compress_ptr_t* cinfo)              \
{                                                                           \
    i386_compress_ptr_t* tmp = ((jmpbuf_helper*)cinfo->client_data)->compress;  \
    wrapCompressStruct(tmp, cinfo);                                         \
    RunFunction_helper2(my_prepare_for_pass_fct_##A, "p", tmp);             \
    unwrapCompressStruct(cinfo, tmp);                                       \
    tmp->client_data = ((jmpbuf_helper*)cinfo->client_data)->client_data;   \
}
SUPER()
#undef GO
static void* findprepare_for_passFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_prepare_for_pass_fct_##A == (uintptr_t)fct) return my_prepare_for_pass_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_prepare_for_pass_fct_##A == 0) {my_prepare_for_pass_fct_##A = (uintptr_t)fct; return my_prepare_for_pass_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libjpeg.so.62 prepare_for_pass callback\n");
    return NULL;
}
static void* reverse_prepare_for_passFct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_prepare_for_pass_##A == fct) return (void*)my_prepare_for_pass_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, vFp, fct, 0, NULL);
}

// pass_startup
#define GO(A)   \
static uintptr_t my_pass_startup_fct_##A = 0;   \
static void my_pass_startup_##A(j62_compress_ptr_t* cinfo)              \
{                                                                           \
    i386_compress_ptr_t* tmp = ((jmpbuf_helper*)cinfo->client_data)->compress;  \
    wrapCompressStruct(tmp, cinfo);                                         \
    RunFunction_helper2(my_pass_startup_fct_##A, "p", tmp);                 \
    unwrapCompressStruct(cinfo, tmp);                                       \
    tmp->client_data = ((jmpbuf_helper*)cinfo->client_data)->client_data;   \
}
SUPER()
#undef GO
static void* findpass_startupFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_pass_startup_fct_##A == (uintptr_t)fct) return my_pass_startup_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_pass_startup_fct_##A == 0) {my_pass_startup_fct_##A = (uintptr_t)fct; return my_pass_startup_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libjpeg.so.62 pass_startup callback\n");
    return NULL;
}
static void* reverse_pass_startupFct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_pass_startup_##A == fct) return (void*)my_pass_startup_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, vFp, fct, 0, NULL);
}

// finish_pass
#define GO(A)   \
static uintptr_t my_finish_pass_fct_##A = 0;   \
static void my_finish_pass_##A(j62_compress_ptr_t* cinfo)              \
{                                                                           \
    i386_compress_ptr_t* tmp = ((jmpbuf_helper*)cinfo->client_data)->compress; \
    wrapCompressStruct(tmp, cinfo);                                         \
    RunFunction_helper2(my_finish_pass_fct_##A, "p", tmp);                  \
    unwrapCompressStruct(cinfo, tmp);                                       \
    tmp->client_data = ((jmpbuf_helper*)cinfo->client_data)->client_data;   \
}
SUPER()
#undef GO
static void* findfinish_passFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_finish_pass_fct_##A == (uintptr_t)fct) return my_finish_pass_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_finish_pass_fct_##A == 0) {my_finish_pass_fct_##A = (uintptr_t)fct; return my_finish_pass_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libjpeg.so.62 finish_pass callback\n");
    return NULL;
}
static void* reverse_finish_passFct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_finish_pass_##A == fct) return (void*)my_finish_pass_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, vFp, fct, 0, NULL);
}
#undef SUPER

#define SUPER()         \
    GO(error_exit)      \
    GO(emit_message)    \
    GO(output_message)  \
    GO(format_message)  \
    GO(reset_error_mgr)

static void wrapErrorMgr(jpeg62_error_mgr_t* mgr)
{
    if(!mgr)
        return;

    #define GO(A) mgr->A = reverse_##A##Fct(mgr->A);

    SUPER()
    #undef GO
}

static void unwrapErrorMgr(jpeg62_error_mgr_t* mgr)
{
    if(!mgr)
        return;

    #define GO(A)    mgr->A = find##A##Fct(mgr->A);
        
    SUPER()
    #undef GO
}
#undef SUPER

#define SUPER()             \
    GO(alloc_small)         \
    GO(alloc_large)         \
    GO(alloc_sarray)        \
    GO(alloc_barray)        \
    GO(request_virt_sarray) \
    GO(request_virt_barray) \
    GO(realize_virt_arrays) \
    GO(access_virt_sarray)  \
    GO(access_virt_barray)  \
    GO(free_pool)           \
    GO(self_destruct)

static void wrapMemoryMgr(jpeg62_memory_mgr_t* mgr)
{
    if(!mgr || (uintptr_t)mgr<0x1000)
        return;

    #define GO(A) mgr->A = reverse_##A##Fct(mgr->A);

    SUPER()
    #undef GO
}

static void unwrapMemoryMgr(jpeg62_memory_mgr_t* mgr)
{
    if(!mgr || (uintptr_t)mgr<0x1000)
        return;

    #define GO(A)    mgr->A = find##A##Fct(mgr->A);
        
    SUPER()
    #undef GO
}
#undef SUPER

#define SUPER()             \
    GO(init_source)         \
    GO(fill_input_buffer)   \
    GO(skip_input_data)     \
    GO(resync_to_restart)   \
    GO(term_source)

static void wrapSourceMgr(jpeg62_source_mgr_t* mgr)
{
    if(!mgr)
        return;

    #define GO(A) mgr->A = reverse_##A##Fct(mgr->A);

    SUPER()
    #undef GO
}

static void unwrapSourceMgr(jpeg62_source_mgr_t* mgr)
{
    if(!mgr)
        return;

    #define GO(A)    mgr->A = find##A##Fct(mgr->A);
        
    SUPER()
    #undef GO
}
#undef SUPER

#define SUPER()             \
  GO(init_destination)      \
  GO(empty_output_buffer)   \
  GO(term_destination)

static void wrapDestinationMgr(jpeg62_destination_mgr_t* mgr)
{
    if(!mgr)
        return;

    #define GO(A) mgr->A = reverse_##A##Fct(mgr->A);

    SUPER()
    #undef GO
}

static void unwrapDestinationMgr(jpeg62_destination_mgr_t* mgr)
{
    if(!mgr)
        return;

    #define GO(A)    mgr->A = find##A##Fct(mgr->A);
        
    SUPER()
    #undef GO
}
#undef SUPER


#define SUPER() \
    GO(prepare_for_pass)    \
    GO(pass_startup)        \
    GO(finish_pass)
static void wrapCompMaster(jpeg62_comp_master_t* master) {
    if (!master) return;
    #define GO(A) master->A = reverse_##A##Fct(master->A);
    SUPER()
    #undef GO
}

static void unwrapCompMaster(jpeg62_comp_master_t* master) {
    if (!master) return;
    #define GO(A) master->A = find##A##Fct(master->A);
    SUPER()
    #undef GO
}

#undef SUPER

static void wrapCommonStruct(void* s, int type)
{
    wrapErrorMgr(((jpeg62_common_struct_t*)s)->err);
    wrapMemoryMgr(((jpeg62_common_struct_t*)s)->mem);
    if(type==1)
        wrapSourceMgr(((j62_decompress_ptr_t*)s)->src);
}
static void unwrapCommonStruct(void* s, int type)
{
    unwrapErrorMgr(((jpeg62_common_struct_t*)s)->err);
    unwrapMemoryMgr(((jpeg62_common_struct_t*)s)->mem);
    if(type==1)
        unwrapSourceMgr(((j62_decompress_ptr_t*)s)->src);
}

static void wrapCompressStruct(i386_compress_ptr_t* d, j62_compress_ptr_t* s) {
    j62compress_arm_to_i386(d, s);
    wrapCommonStruct(s, 0);
    wrapDestinationMgr(s->dest);
    wrapCompMaster(s->master);
}

static void unwrapCompressStruct(j62_compress_ptr_t* d, i386_compress_ptr_t* s) {
    j62compress_i386_to_arm(d, s);
    unwrapCommonStruct(s, 0);
    unwrapDestinationMgr(d->dest);
    unwrapCompMaster(d->master);
}

EXPORT int my62_jpeg_simd_cpu_support()
{
    return 0x01|0x04|0x08; // MMX/SSE/SSE2
}

EXPORT void* my62_jpeg_std_error(x86emu_t* emu, void* errmgr)
{
    (void)emu;
    jpeg62_error_mgr_t* ret = my->jpeg_std_error(errmgr);

    wrapErrorMgr(ret);

    my->jpeg_std_error(&native_err_mgr);
    ret->error_exit = (void*)AddCheckBridge(my_lib->w.bridge, vFp, native_error_exit, 0, NULL);
    ret->emit_message = (void*)AddCheckBridge(my_lib->w.bridge, vFpi, native_emit_message, 0, NULL);
    ret->output_message = (void*)AddCheckBridge(my_lib->w.bridge, vFp, native_output_message, 0, NULL);
    ret->format_message = (void*)AddCheckBridge(my_lib->w.bridge, vFpp, native_format_message, 0, NULL);
    ret->reset_error_mgr = (void*)AddCheckBridge(my_lib->w.bridge, vFp, native_reset_error_mgr, 0, NULL);

    return ret;
}

#define WRAP(R, A, T)                               \
    jmpbuf_helper helper;                           \
    unwrapCommonStruct(cinfo, T);                   \
    if(setjmp(&helper.jmpbuf)) {                    \
        cinfo->client_data = helper.client_data;    \
        wrapCommonStruct(cinfo, T);                 \
        return (R)R_EAX;                            \
    }                                               \
    helper.client_data = cinfo->client_data;        \
    cinfo->client_data = &helper;                   \
    A;                                              \
    cinfo->client_data = helper.client_data;        \
    wrapCommonStruct(cinfo, T)

#define WRAPC(R, A)                                     \
    jmpbuf_helper helper;                               \
    struct j62_compress_ptr_s tmp;                      \
    unwrapCompressStruct(&tmp, cinfo);                  \
    if(setjmp(&helper.jmpbuf)) {                        \
        tmp.client_data = helper.client_data;           \
        wrapCompressStruct(cinfo, &tmp);                \
        return (R)R_EAX;                                \
    }                                                   \
    helper.client_data = cinfo->client_data;            \
    helper.compress = cinfo;                            \
    tmp.client_data = &helper;                          \
    A;                                                  \
    tmp.client_data = helper.client_data;               \
    wrapCompressStruct(cinfo, &tmp);

EXPORT void my62_jpeg_CreateDecompress(x86emu_t* emu, jpeg62_common_struct_t* cinfo, int version, unsigned long structsize)
{
    // Not using WRAP macro because only err field might be initialized here
    unwrapErrorMgr(cinfo->err);
    jmpbuf_helper helper;
    if(setjmp(&helper.jmpbuf)) {
        cinfo->client_data = helper.client_data;
        wrapErrorMgr(cinfo->err);
        return;
    }
    helper.client_data = cinfo->client_data;
    cinfo->client_data = &helper;
    my->jpeg_CreateDecompress(cinfo, version, structsize);
    cinfo->client_data = helper.client_data;
    wrapCommonStruct(cinfo, 1);
}

EXPORT int my62_jpeg_read_header(x86emu_t* emu, jpeg62_common_struct_t* cinfo, int image)
{
    WRAP(int, int ret = my->jpeg_read_header(cinfo, image), 1);
    return ret;
}

EXPORT int my62_jpeg_start_decompress(x86emu_t* emu, jpeg62_common_struct_t* cinfo)
{
    WRAP(int, int ret = my->jpeg_start_decompress(cinfo), 1);
    return ret;
}

EXPORT uint32_t my62_jpeg_read_scanlines(x86emu_t* emu, jpeg62_common_struct_t* cinfo, void* scanlines, uint32_t maxlines)
{
    WRAP(uint32_t, uint32_t ret = my->jpeg_read_scanlines(cinfo, scanlines, maxlines), 1);
    return ret;
}

EXPORT int my62_jpeg_finish_decompress(x86emu_t* emu, jpeg62_common_struct_t* cinfo)
{
    WRAP(int, int ret = my->jpeg_finish_decompress(cinfo), 1);
    return ret;
}

EXPORT void my62_jpeg_set_marker_processor(x86emu_t* emu, jpeg62_common_struct_t* cinfo, int marker, void* routine)
{
    WRAP(void, my->jpeg_set_marker_processor(cinfo, marker, findjpeg_marker_parser_methodFct(routine)), 0);
}

EXPORT void my62_jpeg_stdio_src(x86emu_t* emu, jpeg62_common_struct_t* cinfo, void* infile) {
    WRAP(void, my->jpeg_stdio_src(cinfo, infile), 0);
}

EXPORT void my62_jpeg_stdio_dest(x86emu_t* emu, i386_compress_ptr_t* cinfo, void* outfile) {
    WRAPC(void, my->jpeg_stdio_dest(&tmp, outfile));
}

EXPORT void my62_jpeg_destroy_decompress(x86emu_t* emu, jpeg62_common_struct_t* cinfo)
{
    // no WRAP macro because we don't want to wrap at the exit
    int unwrapped = 0;
    jmpbuf_helper helper;
    if(setjmp(&helper.jmpbuf)) {
        cinfo->client_data = helper.client_data;
        if(unwrapped)
            wrapCommonStruct(cinfo, 1); // error, so re-wrap
        return;
    }
    unwrapCommonStruct(cinfo, 1);
    unwrapped = 1;
    helper.client_data = cinfo->client_data;
    cinfo->client_data = &helper;
    my->jpeg_destroy_decompress(cinfo);
    cinfo->client_data = helper.client_data;
}

EXPORT void my62_jpeg_CreateCompress(x86emu_t* emu, i386_compress_ptr_t* cinfo, int version, unsigned long structsize)
{
    jmpbuf_helper helper;
    struct j62_compress_ptr_s tmp = {0};
    tmp.err = cinfo->err;
    unwrapErrorMgr(tmp.err);
    if(setjmp(&helper.jmpbuf)) {
        cinfo->client_data = helper.client_data;
        return;
    }
    if(structsize!=sizeof(struct i386_compress_ptr_s)) {
        printf_log(LOG_NONE, "Warning, invalid jpeg62 structuresize for compress (%lu/%u)", structsize, sizeof(struct i386_compress_ptr_s));
    }
    helper.client_data = cinfo->client_data;
    helper.compress = cinfo;
    tmp.client_data = &helper;
    my->jpeg_CreateCompress(&tmp, version, sizeof(tmp));
    tmp.client_data = helper.client_data;
    wrapCompressStruct(cinfo, &tmp);
}

EXPORT void my62_jpeg_destroy_compress(x86emu_t* emu, i386_compress_ptr_t* cinfo)
{
    // no WRAP macro because we don't want to wrap at the exit
    jmpbuf_helper helper;
    struct j62_compress_ptr_s tmp;
    unwrapCompressStruct(&tmp, cinfo);
    if(setjmp(&helper.jmpbuf)) {
        tmp.client_data = helper.client_data;
        wrapCompressStruct(cinfo, &tmp); // error, so re-wrap
        return;
    }
    helper.client_data = cinfo->client_data;
    helper.compress = cinfo;
    tmp.client_data = &helper;
    my->jpeg_destroy_compress(&tmp);
    tmp.client_data = helper.client_data;
    wrapCompressStruct(cinfo, &tmp);
}

EXPORT void my62_jpeg_finish_compress(x86emu_t* emu, i386_compress_ptr_t* cinfo)
{
    WRAPC(void, my->jpeg_finish_compress(&tmp));
}

EXPORT int my62_jpeg_resync_to_restart(x86emu_t* emu, i386_compress_ptr_t* cinfo, int desired)
{
    WRAPC(int, int ret = my->jpeg_resync_to_restart(&tmp, desired));
    return ret;
}

EXPORT void my62_jpeg_set_defaults(x86emu_t* emu, i386_compress_ptr_t* cinfo)
{
    WRAPC(void, my->jpeg_set_defaults(&tmp));
}

EXPORT void my62_jpeg_start_compress(x86emu_t* emu, i386_compress_ptr_t* cinfo, int b)
{
    WRAPC(void, my->jpeg_start_compress(&tmp, b));
}

EXPORT uint32_t my62_jpeg_write_scanlines(x86emu_t* emu, i386_compress_ptr_t* cinfo, void* scanlines, uint32_t maxlines)
{
    WRAPC(uint32_t, uint32_t ret = my->jpeg_write_scanlines(&tmp, scanlines, maxlines));
    return ret;
}

EXPORT void my62_jpeg_set_quality(x86emu_t* emu, i386_compress_ptr_t* cinfo, int quality, int force)
{
    WRAPC(void, my->jpeg_set_quality(&tmp, quality, force));
}

EXPORT void my62_jpeg_mem_dest(x86emu_t* emu, i386_compress_ptr_t* cinfo, void* a, void* b)
{
    WRAPC(void, my->jpeg_mem_dest(&tmp, a, b));
}

EXPORT void my62_jpeg_write_marker(x86emu_t* emu, i386_compress_ptr_t* cinfo, int a, void* b, uint32_t c)
{
    WRAPC(void, my->jpeg_write_marker(&tmp, a, b, c));
}

#undef WRAP
#undef WRAPC

#define CUSTOM_INIT \
    my_bridge = lib->w.bridge;     \
    SETALT(my62_);                      \
    getMy(lib);

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"

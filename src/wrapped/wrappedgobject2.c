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

const char* gobject2Name = "libgobject-2.0.so.0";
#define LIBNAME gobject2

typedef void* (*pFi_t)(int);
typedef void* (*pFip_t)(int, void*);
typedef int (*iFppp_t)(void*, void*, void*);
typedef void* (*pFipp_t)(int, void*, void*);
typedef int (*iFippi_t)(int, void*, void*, int);
typedef int (*iFipppi_t)(int, void*, void*, void*, int);
typedef unsigned long (*LFpppppu_t)(void*, void*, void*, void*, void*, uint32_t);
typedef uint32_t (*uFpiupppp_t)(void*, int, uint32_t, void*, void*, void*, void*);
typedef unsigned long (*LFpiupppp_t)(void*, int, uint32_t, void*, void*, void*, void*);
typedef uint32_t (*uFpiippppiup_t)(void*, int, int, void*, void*, void*, void*, int, uint32_t, void*);
typedef uint32_t (*uFpiiupppiu_t)(void*, int, int, uint32_t, void*, void*, void*, int, uint32_t);
typedef uint32_t (*uFpiiupppiup_t)(void*, int, int, uint32_t, void*, void*, void*, int, uint32_t, void*);
typedef uint32_t (*uFpiiupppiupp_t)(void*, int, int, uint32_t, void*, void*, void*, int, uint32_t, void*, void*);
typedef uint32_t (*uFpiiupppiuppp_t)(void*, int, int, uint32_t, void*, void*, void*, int, uint32_t, void*, void*, void*);

#define SUPER() \
    GO(g_signal_connect_data, LFpppppu_t)       \
    GO(g_boxed_type_register_static, iFppp_t)   \
    GO(g_signal_new, uFpiiupppiu_t)             \
    GO(g_signal_newv, uFpiippppiup_t)           \
    GO(g_signal_new_valist, uFpiippppiup_t)     \
    GO(g_signal_handlers_block_matched, uFpiupppp_t)        \
    GO(g_signal_handlers_unblock_matched, uFpiupppp_t)      \
    GO(g_signal_handlers_disconnect_matched, uFpiupppp_t)   \
    GO(g_signal_handler_find, LFpiupppp_t)      \
    GO(g_object_new, pFip_t)                    \
    GO(g_object_new_valist, pFipp_t)            \
    GO(g_type_register_static, iFippi_t)        \
    GO(g_type_register_fundamental, iFipppi_t)  \
    GO(g_type_value_table_peek, pFi_t)

typedef struct gobject2_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} gobject2_my_t;

void* getGobject2My(library_t* lib)
{
    gobject2_my_t* my = (gobject2_my_t*)calloc(1, sizeof(gobject2_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeGobject2My(void* lib)
{
    gobject2_my_t *my = (gobject2_my_t *)lib;
}

static box86context_t* my_context = NULL;
static int signal_cb(void* a, void* b, void* c, void* d)
{
    // signal can have many signature... so first job is to find the data!
    // hopefully, no callback have more than 4 arguments...
    x86emu_t* emu = NULL;
    int i = 0;
    if(a)
        if(IsCallback(my_context, (x86emu_t*)a)) {
            emu = (x86emu_t*)a;
            i = 1;
        }
    if(!emu && b)
        if(IsCallback(my_context, (x86emu_t*)b)) {
            emu = (x86emu_t*)b;
            i = 2;
        }
    if(!emu && c)
        if(IsCallback(my_context, (x86emu_t*)c)) {
            emu = (x86emu_t*)c;
            i = 3;
        }
    if(!emu && d)
        if(IsCallback(my_context, (x86emu_t*)d)) {
            emu = (x86emu_t*)d;
            i = 4;
        }
    if(!i) {
        printf_log(LOG_NONE, "Warning, GObject2 signal callback but no data found!");
        return 0;
    }
    SetCallbackNArg(emu, i);
    if(i>1)
        SetCallbackArg(emu, 0, a);
    if(i>2)
        SetCallbackArg(emu, 1, b);
    if(i>3)
        SetCallbackArg(emu, 2, c);
    SetCallbackArg(emu, i-1, GetCallbackArg(emu, 7));
    SetCallbackAddress(emu, (uintptr_t)GetCallbackArg(emu, 8));
    return RunCallback(emu);
}
static void signal_delete(void* a, void* b)
{
    x86emu_t* emu = (x86emu_t*)a;
    void* d = (void*)GetCallbackArg(emu, 9);
    if(d) {
        SetCallbackNArg(emu, 2);
        SetCallbackArg(emu, 0, GetCallbackArg(emu, 7));
        SetCallbackArg(emu, 1, b);
        SetCallbackAddress(emu, (uintptr_t)GetCallbackArg(emu, 9));
        RunCallback(emu);
    }
    FreeCallback(emu);
}
EXPORT uintptr_t my_g_signal_connect_data(x86emu_t* emu, void* instance, void* detailed, void* c_handler, void* data, void* closure, uint32_t flags)
{
    library_t * lib = GetLib(emu->context->maplib, gobject2Name);
    gobject2_my_t *my = (gobject2_my_t*)lib->priv.w.p2;

    x86emu_t *cb = AddSmallCallback(emu, (uintptr_t)c_handler, 0, NULL, NULL, NULL, NULL);
    SetCallbackArg(cb, 7, data);
    SetCallbackArg(cb, 8, c_handler);
    SetCallbackArg(cb, 9, closure);
    uintptr_t ret = my->g_signal_connect_data(instance, detailed, signal_cb, cb, signal_delete, flags);
    return ret;
}


#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)

#define GO(A)   \
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
    #define GO(A) if(my_copy_fct_##A == (uintptr_t)fct) return my_copy_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_copy_fct_##A == 0) {my_copy_fct_##A = (uintptr_t)fct; return my_copy_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gobject Boxed Copy callback\n");
    return NULL;
}

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
    #define GO(A) if(my_free_fct_##A == (uintptr_t)fct) return my_free_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_free_fct_##A == 0) {my_free_fct_##A = (uintptr_t)fct; return my_free_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gobject Boxed Free callback\n");
    return NULL;
}
// GSignalAccumulator
#define GO(A)   \
static uintptr_t my_accumulator_fct_##A = 0;   \
static int my_accumulator_##A(void* ihint, void* return_accu, void* handler_return, void* data)     \
{                                       \
    return RunFunction(my_context, my_accumulator_fct_##A, 4, ihint, return_accu, handler_return, data);\
}
SUPER()
#undef GO
static void* findAccumulatorFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_accumulator_fct_##A == (uintptr_t)fct) return my_accumulator_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_accumulator_fct_##A == 0) {my_accumulator_fct_##A = (uintptr_t)fct; return my_accumulator_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gobject Signal Accumulator callback\n");
    return NULL;
}

// GClosureMarshal
#define GO(A)   \
static uintptr_t my_marshal_fct_##A = 0;   \
static int my_marshal_##A(void* closure, void* return_value, uint32_t n, void* values, void* hint, void* data)     \
{                                       \
    return RunFunction(my_context, my_marshal_fct_##A, 6, closure, return_value, n, values, hint, data);\
}
SUPER()
#undef GO
static void* findMarshalFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_marshal_fct_##A == (uintptr_t)fct) return my_marshal_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_marshal_fct_##A == 0) {my_marshal_fct_##A = (uintptr_t)fct; return my_marshal_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gobject Closure Marshal callback\n");
    return NULL;
}

// GTypeValueTable
typedef struct my_GTypeValueTable_s {
  void     (*value_init)         (void* value);
  void     (*value_free)         (void* value);
  void     (*value_copy)         (void* src_value, void* dest_value);
  void*    (*value_peek_pointer) (void* value);
  void*    collect_format;
  void*    (*collect_value)      (void* value, uint32_t n_collect_values, void* collect_values, uint32_t collect_flags);
  void*    lcopy_format;
  void*    (*lcopy_value)        (void* value, uint32_t n_collect_values, void* collect_values, uint32_t collect_flags);
} my_GTypeValueTable_t;

// First the structure GTypeInfo statics, with paired x86 source pointer
#define GO(A) \
static my_GTypeValueTable_t     my_gtypevaluetable_##A = {0};   \
static my_GTypeValueTable_t   *ref_gtypevaluetable_##A = NULL;
SUPER()
#undef GO
// Then the static functions callback that may be used with the structure
#define GO(A)   \
static uintptr_t fct_funcs_value_init_##A = 0;  \
static void my_funcs_value_init_##A(void* value) {   \
    RunFunction(my_context, fct_funcs_value_init_##A, 1, value);    \
}   \
static uintptr_t fct_funcs_value_free_##A = 0;  \
static void my_funcs_value_free_##A(void* value) {   \
    RunFunction(my_context, fct_funcs_value_free_##A, 1, value);    \
}   \
static uintptr_t fct_funcs_value_copy_##A = 0;  \
static void my_funcs_value_copy_##A(void* source, void* dest) {   \
    RunFunction(my_context, fct_funcs_value_copy_##A, 2, source, dest);    \
}   \
static uintptr_t fct_funcs_value_peek_pointer_##A = 0;  \
static void* my_funcs_value_peek_pointer_##A(void* value) {   \
    return (void*)RunFunction(my_context, fct_funcs_value_peek_pointer_##A, 1, value);    \
}   \
static uintptr_t fct_funcs_collect_value_##A = 0;  \
static void* my_funcs_collect_value_##A(void* value, uint32_t n, void* collect, uint32_t flags) {   \
    return (void*)RunFunction(my_context, fct_funcs_collect_value_##A, 4, value, n, collect, flags);    \
}   \
static uintptr_t fct_funcs_lcopy_value_##A = 0;  \
static void* my_funcs_lcopy_value_##A(void* value, uint32_t n, void* collect, uint32_t flags) {   \
    return (void*)RunFunction(my_context, fct_funcs_lcopy_value_##A, 4, value, n, collect, flags);    \
}
SUPER()
#undef GO
// And now the get slot / assign... Taking into account that the desired callback may already be a wrapped one (so unwrapping it)
static my_GTypeValueTable_t* findFreeGTypeValueTable(my_GTypeValueTable_t* fcts)
{
    if(!fcts) return fcts;
    #define GO(A) if(ref_gtypevaluetable_##A == fcts) return &my_gtypevaluetable_##A;
    SUPER()
    #undef GO
    #define GO(A) if(ref_gtypevaluetable_##A == 0) {    \
        ref_gtypevaluetable_##A = fcts;                 \
        my_gtypevaluetable_##A.value_init = (fcts->value_init)?((GetNativeFnc((uintptr_t)fcts->value_init))?GetNativeFnc((uintptr_t)fcts->value_init):my_funcs_value_init_##A):NULL;    \
        fct_funcs_value_init_##A = (uintptr_t)fcts->value_init;                             \
        my_gtypevaluetable_##A.value_free = (fcts->value_free)?((GetNativeFnc((uintptr_t)fcts->value_free))?GetNativeFnc((uintptr_t)fcts->value_free):my_funcs_value_free_##A):NULL;    \
        fct_funcs_value_free_##A = (uintptr_t)fcts->value_free;                             \
        my_gtypevaluetable_##A.value_copy = (fcts->value_copy)?((GetNativeFnc((uintptr_t)fcts->value_copy))?GetNativeFnc((uintptr_t)fcts->value_copy):my_funcs_value_copy_##A):NULL;    \
        fct_funcs_value_copy_##A = (uintptr_t)fcts->value_copy;                             \
        my_gtypevaluetable_##A.value_peek_pointer = (fcts->value_peek_pointer)?((GetNativeFnc((uintptr_t)fcts->value_peek_pointer))?GetNativeFnc((uintptr_t)fcts->value_peek_pointer):my_funcs_value_peek_pointer_##A):NULL;    \
        fct_funcs_value_peek_pointer_##A = (uintptr_t)fcts->value_peek_pointer;             \
        my_gtypevaluetable_##A.collect_format = fcts->collect_format;                       \
        my_gtypevaluetable_##A.collect_value = (fcts->collect_value)?((GetNativeFnc((uintptr_t)fcts->collect_value))?GetNativeFnc((uintptr_t)fcts->collect_value):my_funcs_collect_value_##A):NULL;    \
        fct_funcs_collect_value_##A = (uintptr_t)fcts->collect_value;                       \
        my_gtypevaluetable_##A.lcopy_format = fcts->lcopy_format;                           \
        my_gtypevaluetable_##A.lcopy_value = (fcts->lcopy_value)?((GetNativeFnc((uintptr_t)fcts->lcopy_value))?GetNativeFnc((uintptr_t)fcts->lcopy_value):my_funcs_lcopy_value_##A):NULL;    \
        fct_funcs_lcopy_value_##A = (uintptr_t)fcts->lcopy_value;                           \
        return &my_gtypevaluetable_##A;                 \
    }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gobject2 GTypeValueTable callback\n");
    return NULL;
}


// GTypeInfo
typedef void (*GBaseInitFunc)           (void* g_class);
typedef void (*GBaseFinalizeFunc)       (void* g_class);
typedef void (*GClassInitFunc)          (void* g_class, void* class_data);
typedef void (*GClassFinalizeFunc)      (void* g_class, void* class_data);
typedef void (*GInstanceInitFunc)       (void* instance, void* g_class);

typedef struct my_GTypeInfo_s {
  uint16_t  class_size;
  GBaseInitFunc          base_init;
  GBaseFinalizeFunc      base_finalize;
  GClassInitFunc         class_init;
  GClassFinalizeFunc     class_finalize;
  void*                  class_data;
  uint16_t               instance_size;
  uint16_t               n_preallocs;
  GInstanceInitFunc      instance_init;
  my_GTypeValueTable_t*  value_table;
} my_GTypeInfo_t;
// First the structure my_GTypeInfo_t statics, with paired x86 source pointer
#define GO(A) \
static my_GTypeInfo_t     my_gtypeinfo_##A = {0};   \
static my_GTypeInfo_t   *ref_gtypeinfo_##A = NULL;
SUPER()
#undef GO
// Then the static functions callback that may be used with the structure
#define GO(A)   \
static uintptr_t fct_funcs_base_init_##A = 0;  \
static int my_funcs_base_init_##A(void* g_class) {   \
    return (int)RunFunction(my_context, fct_funcs_base_init_##A, 1, g_class);    \
}   \
static uintptr_t fct_funcs_base_finalize_##A = 0;  \
static int my_funcs_base_finalize_##A(void* g_class) {   \
    return (int)RunFunction(my_context, fct_funcs_base_finalize_##A, 1, g_class);    \
}   \
static uintptr_t fct_funcs_class_init_##A = 0;  \
static int my_funcs_class_init_##A(void* g_class, void* data) {   \
    return (int)RunFunction(my_context, fct_funcs_class_init_##A, 2, g_class, data);    \
}   \
static uintptr_t fct_funcs_class_finalize_##A = 0;  \
static int my_funcs_class_finalize_##A(void* g_class, void* data) {   \
    return (int)RunFunction(my_context, fct_funcs_class_finalize_##A, 2, g_class, data);    \
}   \
static uintptr_t fct_funcs_instance_init_##A = 0;  \
static int my_funcs_instance_init_##A(void* instance, void* data) {   \
    return (int)RunFunction(my_context, fct_funcs_instance_init_##A, 2, instance, data);    \
}

SUPER()
#undef GO
// And now the get slot / assign... Taking into account that the desired callback may already be a wrapped one (so unwrapping it)
static my_GTypeInfo_t* findFreeGTypeInfo(my_GTypeInfo_t* fcts)
{
    if(!fcts) return fcts;
    #define GO(A) if(ref_gtypeinfo_##A == fcts) return &my_gtypeinfo_##A;
    SUPER()
    #undef GO
    #define GO(A) if(ref_gtypeinfo_##A == 0) {          \
        ref_gtypeinfo_##A = fcts;                       \
        my_gtypeinfo_##A.class_size = fcts->class_size; \
        my_gtypeinfo_##A.base_init = (fcts->base_init)?((GetNativeFnc((uintptr_t)fcts->base_init))?GetNativeFnc((uintptr_t)fcts->base_init):my_funcs_base_init_##A):NULL;    \
        fct_funcs_base_init_##A = (uintptr_t)fcts->base_init;           \
        my_gtypeinfo_##A.base_finalize = (fcts->base_finalize)?((GetNativeFnc((uintptr_t)fcts->base_finalize))?GetNativeFnc((uintptr_t)fcts->base_finalize):my_funcs_base_finalize_##A):NULL;    \
        fct_funcs_base_finalize_##A = (uintptr_t)fcts->base_finalize;   \
        my_gtypeinfo_##A.class_init = (fcts->class_init)?((GetNativeFnc((uintptr_t)fcts->class_init))?GetNativeFnc((uintptr_t)fcts->class_init):my_funcs_class_init_##A):NULL;    \
        fct_funcs_class_init_##A = (uintptr_t)fcts->class_init;         \
        my_gtypeinfo_##A.class_finalize = (fcts->class_finalize)?((GetNativeFnc((uintptr_t)fcts->class_finalize))?GetNativeFnc((uintptr_t)fcts->class_finalize):my_funcs_class_finalize_##A):NULL;    \
        fct_funcs_class_finalize_##A = (uintptr_t)fcts->class_finalize; \
        my_gtypeinfo_##A.class_data = fcts->class_data;                 \
        my_gtypeinfo_##A.instance_size = fcts->instance_size;           \
        my_gtypeinfo_##A.n_preallocs = fcts->n_preallocs;               \
        my_gtypeinfo_##A.instance_init = (fcts->instance_init)?((GetNativeFnc((uintptr_t)fcts->instance_init))?GetNativeFnc((uintptr_t)fcts->instance_init):my_funcs_instance_init_##A):NULL;    \
        fct_funcs_instance_init_##A = (uintptr_t)fcts->instance_init;   \
        my_gtypeinfo_##A.value_table = findFreeGTypeValueTable(fcts->value_table);           \
        return &my_gtypeinfo_##A;                       \
    }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for glib2 GTypeInfo callback\n");
    return NULL;
}

#undef SUPER

EXPORT int my_g_boxed_type_register_static(x86emu_t* emu, void* name, void* boxed_copy, void* boxed_free)
{
    library_t * lib = GetLib(emu->context->maplib, gobject2Name);
    gobject2_my_t *my = (gobject2_my_t*)lib->priv.w.p2;
    void* bc = findCopyFct(boxed_copy);
    void* bf = findFreeFct(boxed_free);
    return my->g_boxed_type_register_static(name, bc, bf);
}

EXPORT uint32_t my_g_signal_new(x86emu_t* emu, void* name, int itype, int flags, uint32_t offset, void* acc, void* accu_data, void* marsh, int rtype, uint32_t n, void** b)
{
    library_t * lib = GetLib(emu->context->maplib, gobject2Name);
    gobject2_my_t *my = (gobject2_my_t*)lib->priv.w.p2;
    
    void* cb_acc = findAccumulatorFct(acc);
    void* cb_marsh = findMarshalFct(marsh);
    switch(n) {
        case 0: return my->g_signal_new(name, itype, flags, offset, cb_acc, accu_data, cb_marsh, rtype, n);
        case 1: return ((uFpiiupppiup_t)my->g_signal_new)(name, itype, flags, offset, cb_acc, accu_data, cb_marsh, rtype, n, b[0]);
        case 2: return ((uFpiiupppiupp_t)my->g_signal_new)(name, itype, flags, offset, cb_acc, accu_data, cb_marsh, rtype, n, b[0], b[1]);
        case 3: return ((uFpiiupppiuppp_t)my->g_signal_new)(name, itype, flags, offset, cb_acc, accu_data, cb_marsh, rtype, n, b[0], b[1], b[2]);
        default:
            printf_log(LOG_NONE, "Warning, gobject g_signal_new called with too many parameters (%d)\n", n);
    }
    return ((uFpiiupppiuppp_t)my->g_signal_new)(name, itype, flags, offset, cb_acc, accu_data, cb_marsh, rtype, n, b[0], b[1], b[2]);
}

EXPORT uint32_t my_g_signal_newv(x86emu_t* emu, void* name, int itype, int flags, void* closure, void* acc, void* accu_data, void* marsh, int rtype, uint32_t n, void* types)
{
    library_t * lib = GetLib(emu->context->maplib, gobject2Name);
    gobject2_my_t *my = (gobject2_my_t*)lib->priv.w.p2;

    return my->g_signal_newv(name, itype, flags, closure, findAccumulatorFct(acc), accu_data, findMarshalFct(marsh), rtype, n, types);
}

EXPORT uint32_t my_g_signal_new_valist(x86emu_t* emu, void* name, int itype, int flags, void* closure, void* acc, void* accu_data, void* marsh, int rtype, uint32_t n, void* b)
{
    library_t * lib = GetLib(emu->context->maplib, gobject2Name);
    gobject2_my_t *my = (gobject2_my_t*)lib->priv.w.p2;

    return my->g_signal_new_valist(name, itype, flags, closure, findAccumulatorFct(acc), accu_data, findMarshalFct(marsh), rtype, n, b);
}

EXPORT uint32_t my_g_signal_handlers_block_matched(x86emu_t* emu, void* instance, int mask, uint32_t signal, void* detail, void* closure, void* fnc, void* data)
{
    library_t * lib = GetLib(emu->context->maplib, gobject2Name);
    gobject2_my_t *my = (gobject2_my_t*)lib->priv.w.p2;

    // NOTE that I have no idea of the fnc signature!...
    if (fnc) printf_log(LOG_NONE, "Warning, gobject g_signal_handlers_block_matched called with non null function \n");
    fnc = findMarshalFct(fnc);  //... just in case
    return my->g_signal_handlers_block_matched(instance, mask, signal, detail, closure, fnc, data);
}

EXPORT uint32_t my_g_signal_handlers_unblock_matched(x86emu_t* emu, void* instance, int mask, uint32_t signal, void* detail, void* closure, void* fnc, void* data)
{
    library_t * lib = GetLib(emu->context->maplib, gobject2Name);
    gobject2_my_t *my = (gobject2_my_t*)lib->priv.w.p2;

    // NOTE that I have no idea of the fnc signature!...
    if (fnc) printf_log(LOG_NONE, "Warning, gobject g_signal_handlers_unblock_matched called with non null function \n");
    fnc = findMarshalFct(fnc);  //... just in case
    return my->g_signal_handlers_unblock_matched(instance, mask, signal, detail, closure, fnc, data);
}

EXPORT uint32_t my_g_signal_handlers_disconnect_matched(x86emu_t* emu, void* instance, int mask, uint32_t signal, void* detail, void* closure, void* fnc, void* data)
{
    library_t * lib = GetLib(emu->context->maplib, gobject2Name);
    gobject2_my_t *my = (gobject2_my_t*)lib->priv.w.p2;

    // NOTE that I have no idea of the fnc signature!...
    if (fnc) printf_log(LOG_NONE, "Warning, gobject g_signal_handlers_disconnect_matched called with non null function \n");
    fnc = findMarshalFct(fnc);  //... just in case
    return my->g_signal_handlers_disconnect_matched(instance, mask, signal, detail, closure, fnc, data);
}

EXPORT unsigned long my_g_signal_handler_find(x86emu_t* emu, void* instance, int mask, uint32_t signal, void* detail, void* closure, void* fnc, void* data)
{
    library_t * lib = GetLib(emu->context->maplib, gobject2Name);
    gobject2_my_t *my = (gobject2_my_t*)lib->priv.w.p2;

    // NOTE that I have no idea of the fnc signature!...
    if (fnc) printf_log(LOG_NONE, "Warning, gobject g_signal_handler_find called with non null function \n");
    fnc = findMarshalFct(fnc);  //... just in case
    return my->g_signal_handler_find(instance, mask, signal, detail, closure, fnc, data);
}

EXPORT void* my_g_object_new(x86emu_t* emu, int type, void* first, void* b)
{
    library_t * lib = GetLib(emu->context->maplib, gobject2Name);
    gobject2_my_t *my = (gobject2_my_t*)lib->priv.w.p2;

    if(first)
        return my->g_object_new_valist(type, first, b);
    return my->g_object_new(type, first);
}

EXPORT int my_g_type_register_static(x86emu_t* emu, int parent, void* name, my_GTypeInfo_t* info, int flags)
{
    library_t * lib = GetLib(emu->context->maplib, gobject2Name);
    gobject2_my_t *my = (gobject2_my_t*)lib->priv.w.p2;

    return my->g_type_register_static(parent, name, findFreeGTypeInfo(info), flags);
}

EXPORT int my_g_type_register_fundamental(x86emu_t* emu, int parent, void* name, my_GTypeInfo_t* info, void* finfo, int flags)
{
    library_t * lib = GetLib(emu->context->maplib, gobject2Name);
    gobject2_my_t *my = (gobject2_my_t*)lib->priv.w.p2;

    return my->g_type_register_fundamental(parent, name, findFreeGTypeInfo(info), finfo, flags);
}

#define CUSTOM_INIT \
    my_context = box86;    \
    lib->priv.w.p2 = getGobject2My(lib); \
    lib->priv.w.needed = 1; \
    lib->priv.w.neededlibs = (char**)calloc(lib->priv.w.needed, sizeof(char*)); \
    lib->priv.w.neededlibs[0] = strdup("libglib-2.0.so.0");

#define CUSTOM_FINI \
    freeGobject2My(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"


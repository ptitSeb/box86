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
#include "box86context.h"
#include "librarian.h"
#include "myalign.h"

const char* monounityName = "libmonounity.so";
#define LIBNAME monounity

typedef void (*vFv_t)();
typedef void (*vFp_t)(void*);
typedef void (*vFpip_t)(void*, int32_t, void*);
typedef void* (*pFpupppp_t)(void*, uint32_t, void*, void*, void*, void*);

typedef struct monounity_my_s {
    // functions
    pFpupppp_t    mono_unity_liveness_allocate_struct;
    pFpupppp_t    mono_unity_liveness_calculation_begin;
    vFp_t         mono_unity_liveness_free_struct;
} monounity_my_t;

void* getMonoUnityMy(library_t* lib)
{
    monounity_my_t* my = (monounity_my_t*)calloc(1, sizeof(monounity_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    GO(mono_unity_liveness_allocate_struct, pFpupppp_t)
    GO(mono_unity_liveness_calculation_begin, pFpupppp_t)
    GO(mono_unity_liveness_free_struct, vFp_t)
    #undef GO
    return my;
}

void freeMonoUnityMy(void* lib)
{
    monounity_my_t *my = (monounity_my_t *)lib;
}

static void my_register_object_callback(void* arr, int size, void* callback_userdata)
{
    if(callback_userdata) {
        x86emu_t* emu = (x86emu_t*)callback_userdata;
        SetCallbackArg(emu, 0, arr);
        SetCallbackArg(emu, 1, (void*)size);
        RunCallback(emu);
    }
}

static x86emu_t* cb_onstart = NULL;
static x86emu_t* cb_onstop = NULL;
static void my_onstart()
{
    if(cb_onstart)
        RunCallback(cb_onstart);
}
static void my_onstop()
{
    if(cb_onstop)
        RunCallback(cb_onstop);
}
typedef struct _LS_s
{
        int32_t     first_index_in_all_objects;
        void*       all_objects;
        void*       filter;
        void*       process_array;
        uint32_t    initial_alloc_count;
        void*       callback_userdata;
        void*       filter_callback;
        void*       onWorldStartCallback;
        void*       onWorldStopCallback;
} _LS_t;

EXPORT void* my_mono_unity_liveness_allocate_struct(x86emu_t*emu, void* filter, uint32_t max_count, void* callback, void* userdata, void* onstartcb, void* onstopcb)
{
    monounity_my_t* my = (monounity_my_t*)emu->context->monounity->priv.w.p2;
    if((onstartcb && cb_onstart) || (onstopcb && cb_onstop)) {
        printf_log(LOG_NONE, "Warning, using mono_unity_liveness_allocate_struct with OnStart/OnStop a second time!\n");
    }
    if(onstartcb) {
        if(cb_onstart)
            FreeCallback(cb_onstart);
        cb_onstart = AddSmallCallback(emu, (uintptr_t)onstartcb, 0, NULL, NULL, NULL, NULL);
    }
    if(onstopcb) {
        if(cb_onstop)
            FreeCallback(cb_onstop);
        cb_onstop = AddSmallCallback(emu, (uintptr_t)onstopcb, 0, NULL, NULL, NULL, NULL);
    }
    x86emu_t *cb = AddCallback(emu, (uintptr_t)callback, 3, NULL, NULL, userdata, NULL);

    return my->mono_unity_liveness_allocate_struct(filter, max_count, my_register_object_callback, cb, (onstartcb)?my_onstart:NULL, (onstopcb)?my_onstop:NULL);
}
EXPORT void* my_mono_unity_liveness_calculation_begin(x86emu_t*emu, void* filter, uint32_t max_count, void* callback, void* userdata, void* onstartcb, void* onstopcb)
{
    monounity_my_t* my = (monounity_my_t*)emu->context->monounity->priv.w.p2;
    if((onstartcb && cb_onstart) || (onstopcb && cb_onstop)) {
        printf_log(LOG_NONE, "Warning, using mono_unity_liveness_calculation_begin with OnStart/OnStop a second time!\n");
    }
    if(onstartcb) {
        if(cb_onstart)
            FreeCallback(cb_onstart);
        cb_onstart = AddSmallCallback(emu, (uintptr_t)onstartcb, 0, NULL, NULL, NULL, NULL);
    }
    if(onstopcb) {
        if(cb_onstop)
            FreeCallback(cb_onstop);
        cb_onstop = AddSmallCallback(emu, (uintptr_t)onstopcb, 0, NULL, NULL, NULL, NULL);
    }
    x86emu_t *cb = AddCallback(emu, (uintptr_t)callback, 3, NULL, NULL, userdata, NULL);

    return my->mono_unity_liveness_calculation_begin(filter, max_count, my_register_object_callback, cb, (onstartcb)?my_onstart:NULL, (onstopcb)?my_onstop:NULL);
}
EXPORT void my_mono_unity_liveness_free_struct(x86emu_t* emu, void* livenessstate)
{
    monounity_my_t* my = (monounity_my_t*)emu->context->monounity->priv.w.p2;
    _LS_t* state = (_LS_t*) livenessstate;
    FreeCallback(state->callback_userdata);
    if(state->onWorldStartCallback) {
        FreeCallback(cb_onstart);
        cb_onstart = NULL;
    }
    if(state->onWorldStopCallback) {
        FreeCallback(cb_onstart);
        cb_onstart = NULL;
    }
    my->mono_unity_liveness_free_struct(livenessstate);
}

#define CUSTOM_INIT \
    box86->monounity = lib; \
    lib->priv.w.p2 = getMonoUnityMy(lib);

#define CUSTOM_FINI \
    freeMonoUnityMy(lib->priv.w.p2); \
    free(lib->priv.w.p2); \
    lib->context->monounity = NULL;

#include "wrappedlib_init.h"


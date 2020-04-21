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


const char* libvorbisName = "libvorbis.so.0";
#define LIBNAME libvorbis

typedef void (*vFp_t)(void*);
typedef int32_t (*iFp_t)(void*);
typedef int32_t (*iFpi_t)(void*, int32_t);
typedef int32_t (*iFpp_t)(void*, void*);
typedef int32_t (*iFppppp_t)(void*, void*, void*, void*, void*);
typedef void* (*pFpi_t)(void*, int32_t);

typedef struct vorbis_my_s {
    // functions
    iFpp_t  vorbis_block_init;
    iFp_t   vorbis_block_clear;
    iFpp_t  vorbis_analysis;
    iFpp_t  vorbis_analysis_blockout;
    pFpi_t  vorbis_analysis_buffer;
    iFppppp_t  vorbis_analysis_headerout;
    iFpp_t  vorbis_analysis_init;
    iFpi_t  vorbis_analysis_wrote;
    iFp_t   vorbis_bitrate_addblock;
    iFpp_t  vorbis_bitrate_flushpacket;
    vFp_t   vorbis_dsp_clear;
    iFpp_t  vorbis_synthesis;
    iFpp_t  vorbis_synthesis_blockin;
    iFpp_t  vorbis_synthesis_init;
    iFpp_t  vorbis_synthesis_lapout;
    iFpp_t  vorbis_synthesis_pcmout;
    iFpi_t  vorbis_synthesis_read;
    iFp_t   vorbis_synthesis_restart;
    iFpp_t  vorbis_synthesis_trackonly;
    pFpi_t  vorbis_window;
} vorbis_my_t;

void* getVorbisMy(library_t* lib)
{
    vorbis_my_t* my = (vorbis_my_t*)calloc(1, sizeof(vorbis_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    GO(vorbis_block_clear, iFp_t)
    GO(vorbis_block_init, iFpp_t)
    GO(vorbis_analysis, iFpp_t)
    GO(vorbis_analysis_blockout, iFpp_t)
    GO(vorbis_analysis_buffer, pFpi_t)
    GO(vorbis_analysis_headerout, iFppppp_t)
    GO(vorbis_analysis_init, iFpp_t)
    GO(vorbis_analysis_wrote, iFpi_t)
    GO(vorbis_bitrate_addblock, iFp_t)
    GO(vorbis_bitrate_flushpacket, iFpp_t)
    GO(vorbis_dsp_clear, vFp_t)
    GO(vorbis_synthesis, iFpp_t)
    GO(vorbis_synthesis_blockin, iFpp_t)
    GO(vorbis_synthesis_init, iFpp_t)
    GO(vorbis_synthesis_lapout, iFpp_t)
    GO(vorbis_synthesis_pcmout, iFpp_t)
    GO(vorbis_synthesis_read, iFpi_t)
    GO(vorbis_synthesis_restart, iFp_t)
    GO(vorbis_synthesis_trackonly, iFpp_t)
    GO(vorbis_window, pFpi_t)
    #undef GO
    return my;
}

void freeVorbisMy(void* lib)
{
    //vorbis_my_t *my = (vorbis_my_t *)lib;
}

#ifndef NOALIGN

int EXPORT my_vorbis_block_clear(x86emu_t *emu, void * vb)
{
    vorbis_my_t* my = (vorbis_my_t*)emu->context->vorbis->priv.w.p2;
    vorbis_block block;
    AlignVorbisBlock(&block, vb);
    int ret = my->vorbis_block_clear(&block);
    UnalignVorbisBlock(vb, &vb);
    return ret;
}

int EXPORT my_vorbis_block_init(x86emu_t *emu, void * v, void* vb)
{
    vorbis_my_t* my = (vorbis_my_t*)emu->context->vorbis->priv.w.p2;
    vorbis_block block;
    vorbis_dsp_state state;
    AlignVorbisDspState(&state, v);
    AlignVorbisBlock(&block, vb);   // useless?
    int ret = my->vorbis_block_init(&state, &block);
    UnalignVorbisBlock(vb, &block);
    UnalignVorbisDspState(v, &state);   // just in case?
    return ret;
}

int EXPORT my_vorbis_analysis(x86emu_t *emu, void * vb, void* op)
{
    vorbis_my_t* my = (vorbis_my_t*)emu->context->vorbis->priv.w.p2;
    vorbis_block block;
    AlignVorbisBlock(&block, vb);
    int ret = my->vorbis_analysis(&block, op);
    UnalignVorbisBlock(vb, &block);
    return ret;
}

int EXPORT my_vorbis_analysis_blockout(x86emu_t *emu, void * v, void* vb)
{
    vorbis_my_t* my = (vorbis_my_t*)emu->context->vorbis->priv.w.p2;
    vorbis_block block;
    vorbis_dsp_state state;
    AlignVorbisDspState(&state, v);
    AlignVorbisBlock(&block, vb);
    int ret = my->vorbis_analysis_blockout(&state, &block);
    UnalignVorbisBlock(vb, &block);  // is it usefull?
    UnalignVorbisDspState(v, &state);
    return ret;
}

void EXPORT *my_vorbis_analysis_buffer(x86emu_t *emu, void * v, int32_t vals)
{
    vorbis_my_t* my = (vorbis_my_t*)emu->context->vorbis->priv.w.p2;
    vorbis_dsp_state state;
    AlignVorbisDspState(&state, v);
    void* ret = my->vorbis_analysis_buffer(&state, vals);
    UnalignVorbisDspState(v, &state);
    return ret;
}

int EXPORT my_vorbis_analysis_headerout(x86emu_t *emu, void * v, void* vc, void* op, void* op_comm, void* op_code)
{
    vorbis_my_t* my = (vorbis_my_t*)emu->context->vorbis->priv.w.p2;
    vorbis_dsp_state state;
    AlignVorbisDspState(&state, v);
    int32_t ret = my->vorbis_analysis_headerout(&state, vc, op, op_comm, op_code);
    UnalignVorbisDspState(v, &state);
    return ret;
}

int EXPORT my_vorbis_analysis_init(x86emu_t* emu, void* v, void* vi)
{
    vorbis_my_t* my = (vorbis_my_t*)emu->context->vorbis->priv.w.p2;
    vorbis_dsp_state state;
    AlignVorbisDspState(&state, v); // usefull?
    int ret = my->vorbis_analysis_init(&state, vi);
    UnalignVorbisDspState(v, &state);
    return ret;
}

int EXPORT my_vorbis_analysis_wrote(x86emu_t *emu, void * v, int32_t samples)
{
    vorbis_my_t* my = (vorbis_my_t*)emu->context->vorbis->priv.w.p2;
    vorbis_dsp_state state;
    AlignVorbisDspState(&state, v);
    int ret = my->vorbis_analysis_wrote(&state, samples);
    UnalignVorbisDspState(v, &state);
    return ret;
}

int EXPORT my_vorbis_bitrate_addblock(x86emu_t *emu, void * vb)
{
    vorbis_my_t* my = (vorbis_my_t*)emu->context->vorbis->priv.w.p2;
    vorbis_block block;
    AlignVorbisBlock(&block, vb);
    int ret = my->vorbis_bitrate_addblock(&block);
    UnalignVorbisBlock(vb, &block);
    return ret;
}

int EXPORT my_vorbis_bitrate_flushpacket(x86emu_t* emu, void* v, void* op)
{
    vorbis_my_t* my = (vorbis_my_t*)emu->context->vorbis->priv.w.p2;
    vorbis_dsp_state state;
    AlignVorbisDspState(&state, v);
    int ret = my->vorbis_bitrate_flushpacket(&state, op);
    UnalignVorbisDspState(v, &state);
    return ret;
}

void EXPORT my_vorbis_dsp_clear(x86emu_t *emu, void * v)
{
    vorbis_my_t* my = (vorbis_my_t*)emu->context->vorbis->priv.w.p2;
    vorbis_dsp_state state;
    AlignVorbisDspState(&state, v);
    my->vorbis_dsp_clear(&state);
    UnalignVorbisDspState(v, &state);
}

int EXPORT my_vorbis_synthesis(x86emu_t *emu, void * vb, void* op)
{
    vorbis_my_t* my = (vorbis_my_t*)emu->context->vorbis->priv.w.p2;
    vorbis_block block;
    AlignVorbisBlock(&block, vb);
    int ret = my->vorbis_synthesis(&block, op);
    UnalignVorbisBlock(vb, &block);
    return ret;
}

int EXPORT my_vorbis_synthesis_blockin(x86emu_t *emu, void * v, void* vb)
{
    vorbis_my_t* my = (vorbis_my_t*)emu->context->vorbis->priv.w.p2;
    vorbis_block block;
    vorbis_dsp_state state;
    AlignVorbisDspState(&state, v);
    AlignVorbisBlock(&block, vb);
    int ret = my->vorbis_synthesis_blockin(&state, &block);
    UnalignVorbisBlock(vb, &block);  // is it usefull?
    UnalignVorbisDspState(v, &state);
    return ret;
}

int EXPORT my_vorbis_synthesis_init(x86emu_t *emu, void * v, void* vi)
{
    vorbis_my_t* my = (vorbis_my_t*)emu->context->vorbis->priv.w.p2;
    vorbis_dsp_state state;
    AlignVorbisDspState(&state, v); // useless?
    int ret = my->vorbis_synthesis_init(&state, vi);
    UnalignVorbisDspState(v, &state);
    return ret;
}

int EXPORT my_vorbis_synthesis_lapout(x86emu_t *emu, void* v, void* pcm)
{
    vorbis_my_t* my = (vorbis_my_t*)emu->context->vorbis->priv.w.p2;
    vorbis_dsp_state state;
    AlignVorbisDspState(&state, v);
    int ret = my->vorbis_synthesis_lapout(&state, pcm);
    UnalignVorbisDspState(v, &state);
    return ret;
}

int EXPORT my_vorbis_synthesis_pcmout(x86emu_t *emu, void * v, void* pcm)
{
    vorbis_my_t* my = (vorbis_my_t*)emu->context->vorbis->priv.w.p2;
    vorbis_dsp_state state;
    AlignVorbisDspState(&state, v);
    int ret = my->vorbis_synthesis_pcmout(&state, pcm);
    UnalignVorbisDspState(v, &state);
    return ret;
}

int EXPORT my_vorbis_synthesis_read(x86emu_t *emu, void * v, int32_t samples)
{
    vorbis_my_t* my = (vorbis_my_t*)emu->context->vorbis->priv.w.p2;
    vorbis_dsp_state state;
    AlignVorbisDspState(&state, v);
    int ret = my->vorbis_synthesis_read(&state, samples);
    UnalignVorbisDspState(v, &state);
    return ret;
}

int EXPORT my_vorbis_synthesis_restart(x86emu_t *emu, void * v)
{
    vorbis_my_t* my = (vorbis_my_t*)emu->context->vorbis->priv.w.p2;
    vorbis_dsp_state state;
    AlignVorbisDspState(&state, v);
    int ret = my->vorbis_synthesis_restart(&state);
    UnalignVorbisDspState(v, &state);
    return ret;
}

int EXPORT my_vorbis_synthesis_trackonly(x86emu_t *emu, void * vb, void* op)
{
    vorbis_my_t* my = (vorbis_my_t*)emu->context->vorbis->priv.w.p2;
    vorbis_block block;
    AlignVorbisBlock(&block, vb);
    int ret = my->vorbis_synthesis_trackonly(&block, op);
    UnalignVorbisBlock(vb, &block);
    return ret;
}

// this one seems wrong (use emulated vorbisfile to debug)
EXPORT void* my_vorbis_window(x86emu_t *emu, void* v, int W)
{
    vorbis_my_t* my = (vorbis_my_t*)emu->context->vorbis->priv.w.p2;
    vorbis_dsp_state state;
    AlignVorbisDspState(&state, v);
    void* ret = my->vorbis_window(&state, W);
    UnalignVorbisDspState(v, &state);
    return ret;
}

#endif  //!NOALIGN

#define CUSTOM_INIT \
    box86->vorbis = lib; \
    lib->priv.w.p2 = getVorbisMy(lib);

#define CUSTOM_FINI \
    freeVorbisMy(lib->priv.w.p2); \
    free(lib->priv.w.p2); \
    lib->context->vorbis = NULL;

#include "wrappedlib_init.h"


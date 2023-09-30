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
#include "box86context.h"
#include "librarian.h"
#include "myalign.h"

#ifdef ANDROID
    const char* libvorbisName = "libvorbis.so";
#else
    const char* libvorbisName = "libvorbis.so.0";
#endif

#define LIBNAME libvorbis

#define ADDED_FUNCTIONS()           \

#include "generated/wrappedlibvorbistypes.h"

#include "wrappercallback.h"

#ifndef NOALIGN

int EXPORT my_vorbis_block_clear(x86emu_t *emu, void * vb)
{
    vorbis_block block;
    AlignVorbisBlock(&block, vb);
    int ret = my->vorbis_block_clear(&block);
    UnalignVorbisBlock(vb, &vb);
    return ret;
}

int EXPORT my_vorbis_block_init(x86emu_t *emu, void * v, void* vb)
{
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
    vorbis_block block;
    AlignVorbisBlock(&block, vb);
    int ret = my->vorbis_analysis(&block, op);
    UnalignVorbisBlock(vb, &block);
    return ret;
}

int EXPORT my_vorbis_analysis_blockout(x86emu_t *emu, void * v, void* vb)
{
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
    vorbis_dsp_state state;
    AlignVorbisDspState(&state, v);
    void* ret = my->vorbis_analysis_buffer(&state, vals);
    UnalignVorbisDspState(v, &state);
    return ret;
}

int EXPORT my_vorbis_analysis_headerout(x86emu_t *emu, void * v, void* vc, void* op, void* op_comm, void* op_code)
{
    vorbis_dsp_state state;
    AlignVorbisDspState(&state, v);
    int32_t ret = my->vorbis_analysis_headerout(&state, vc, op, op_comm, op_code);
    UnalignVorbisDspState(v, &state);
    return ret;
}

int EXPORT my_vorbis_analysis_init(x86emu_t* emu, void* v, void* vi)
{
    vorbis_dsp_state state;
    AlignVorbisDspState(&state, v); // usefull?
    int ret = my->vorbis_analysis_init(&state, vi);
    UnalignVorbisDspState(v, &state);
    return ret;
}

int EXPORT my_vorbis_analysis_wrote(x86emu_t *emu, void * v, int32_t samples)
{
    vorbis_dsp_state state;
    AlignVorbisDspState(&state, v);
    int ret = my->vorbis_analysis_wrote(&state, samples);
    UnalignVorbisDspState(v, &state);
    return ret;
}

int EXPORT my_vorbis_bitrate_addblock(x86emu_t *emu, void * vb)
{
    vorbis_block block;
    AlignVorbisBlock(&block, vb);
    int ret = my->vorbis_bitrate_addblock(&block);
    UnalignVorbisBlock(vb, &block);
    return ret;
}

int EXPORT my_vorbis_bitrate_flushpacket(x86emu_t* emu, void* v, void* op)
{
    vorbis_dsp_state state;
    AlignVorbisDspState(&state, v);
    int ret = my->vorbis_bitrate_flushpacket(&state, op);
    UnalignVorbisDspState(v, &state);
    return ret;
}

void EXPORT my_vorbis_dsp_clear(x86emu_t *emu, void * v)
{
    vorbis_dsp_state state;
    AlignVorbisDspState(&state, v);
    my->vorbis_dsp_clear(&state);
    UnalignVorbisDspState(v, &state);
}

int EXPORT my_vorbis_synthesis(x86emu_t *emu, void * vb, void* op)
{
    vorbis_block block;
    AlignVorbisBlock(&block, vb);
    int ret = my->vorbis_synthesis(&block, op);
    UnalignVorbisBlock(vb, &block);
    return ret;
}

int EXPORT my_vorbis_synthesis_blockin(x86emu_t *emu, void * v, void* vb)
{
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
    vorbis_dsp_state state;
    AlignVorbisDspState(&state, v); // useless?
    int ret = my->vorbis_synthesis_init(&state, vi);
    UnalignVorbisDspState(v, &state);
    return ret;
}

int EXPORT my_vorbis_synthesis_lapout(x86emu_t *emu, void* v, void* pcm)
{
    vorbis_dsp_state state;
    AlignVorbisDspState(&state, v);
    int ret = my->vorbis_synthesis_lapout(&state, pcm);
    UnalignVorbisDspState(v, &state);
    return ret;
}

int EXPORT my_vorbis_synthesis_pcmout(x86emu_t *emu, void * v, void* pcm)
{
    vorbis_dsp_state state;
    AlignVorbisDspState(&state, v);
    int ret = my->vorbis_synthesis_pcmout(&state, pcm);
    UnalignVorbisDspState(v, &state);
    return ret;
}

int EXPORT my_vorbis_synthesis_read(x86emu_t *emu, void * v, int32_t samples)
{
    vorbis_dsp_state state;
    AlignVorbisDspState(&state, v);
    int ret = my->vorbis_synthesis_read(&state, samples);
    UnalignVorbisDspState(v, &state);
    return ret;
}

int EXPORT my_vorbis_synthesis_restart(x86emu_t *emu, void * v)
{
    vorbis_dsp_state state;
    AlignVorbisDspState(&state, v);
    int ret = my->vorbis_synthesis_restart(&state);
    UnalignVorbisDspState(v, &state);
    return ret;
}

int EXPORT my_vorbis_synthesis_trackonly(x86emu_t *emu, void * vb, void* op)
{
    vorbis_block block;
    AlignVorbisBlock(&block, vb);
    int ret = my->vorbis_synthesis_trackonly(&block, op);
    UnalignVorbisBlock(vb, &block);
    return ret;
}

// this one seems wrong (use emulated vorbisfile to debug)
EXPORT void* my_vorbis_window(x86emu_t *emu, void* v, int W)
{
    vorbis_dsp_state state;
    AlignVorbisDspState(&state, v);
    void* ret = my->vorbis_window(&state, W);
    UnalignVorbisDspState(v, &state);
    return ret;
}

#endif  //!NOALIGN

#define CUSTOM_INIT \
    getMy(lib);

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"


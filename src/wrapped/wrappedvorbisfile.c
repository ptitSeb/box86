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
#include "bridge.h"

#ifdef PANDORA
const char* vorbisfileNameAlt = "libvorbisifile.so.1";
const char* vorbisfileNameReg = "libvorbisfile.so.3";
const char* vorbisfileName = NULL;
#else
const char* vorbisfileName = "libvorbisfile.so.3";
#endif
#define LIBNAME vorbisfile

typedef void*   (*pFpi_t)(void*, int32_t);
typedef int32_t (*iFp_t)(void*);
typedef int32_t (*iFpi_t)(void*, int32_t);
typedef int32_t (*iFpp_t)(void*, void*);
typedef int32_t (*iFpd_t)(void*, double);
typedef int32_t (*iFpI_t)(void*, int64_t);
typedef int32_t (*iFppip_t)(void*, void*, int32_t, void*);
typedef int32_t (*iFpppi_t)(void*, void*, void*, int32_t);
typedef int32_t (*iFpppi_t)(void*, void*, void*, int32_t);
typedef int32_t (*iFppplC_t)(void*, void*, void*, long, ov_callbacks);
typedef int32_t (*iFppiiiip_t)(void*, void*, int32_t, int32_t, int32_t, int32_t, void*);
typedef int64_t (*IFp_t)(void*);
typedef int64_t (*IFpi_t)(void*, int32_t);
typedef double  (*dFp_t)(void*);
typedef double  (*dFpi_t)(void*, int32_t);

#define SUPER() \
    GO(ov_bitrate, iFpi_t)              \
    GO(ov_bitrate_instant, iFp_t)       \
    GO(ov_clear, iFp_t)                 \
    GO(ov_comment, iFpi_t)              \
    GO(ov_crosslap, iFpp_t)             \
    GO(ov_fopen, iFpp_t)                \
    GO(ov_halfrate, iFpi_t)             \
    GO(ov_halfrate_p, iFp_t)            \
    GO(ov_info, pFpi_t)                 \
    GO(ov_open, iFpppi_t)               \
    GO(ov_open_callbacks, iFppplC_t)    \
    GO(ov_pcm_seek, iFpI_t)             \
    GO(ov_pcm_seek_lap, iFpI_t)         \
    GO(ov_pcm_seek_page, iFpI_t)        \
    GO(ov_pcm_seek_page_lap, iFpI_t)    \
    GO(ov_pcm_tell, IFp_t)              \
    GO(ov_pcm_total, IFpi_t)            \
    GO(ov_raw_seek, iFpi_t)             \
    GO(ov_raw_seek_lap, iFpi_t)         \
    GO(ov_raw_tell, IFp_t)              \
    GO(ov_raw_total, IFpi_t)            \
    GO(ov_read, iFppiiiip_t)            \
    GO(ov_read_float,iFppip_t)          \
    GO(ov_seekable, iFp_t)              \
    GO(ov_serialnumber, iFpi_t)         \
    GO(ov_streams, iFp_t)               \
    GO(ov_test, iFpppi_t)               \
    GO(ov_test_open, iFp_t)             \
    GO(ov_time_seek, iFpd_t)            \
    GO(ov_time_seek_lap, iFpd_t)        \
    GO(ov_time_seek_page, iFpd_t)       \
    GO(ov_time_seek_page_lap, iFpd_t)   \
    GO(ov_time_tell, dFp_t)             \
    GO(ov_time_total, dFpi_t)           \

#include "wrappercallback.h"

int32_t my_ov_open_callbacks(x86emu_t* emu, void* datasource, void* vf, void* initial, long ibytes, void* read, void* seek, void* close, void* tell);

#ifndef NOALIGN

EXPORT int32_t my_ov_bitrate(x86emu_t* emu, void* vf, int32_t i) {
    OggVorbis oggvorbis;
    AlignOggVorbis(&oggvorbis, vf);
    return my->ov_bitrate(&oggvorbis, i);
}
EXPORT int32_t my_ov_bitrate_instant(x86emu_t* emu, void* vf) {
    OggVorbis oggvorbis;
    AlignOggVorbis(&oggvorbis, vf);
    int32_t ret = my->ov_bitrate_instant(&oggvorbis);
    UnalignOggVorbis(vf, &oggvorbis);
    return ret;
}
EXPORT int32_t my_ov_clear(x86emu_t* emu, void* vf) {
    OggVorbis oggvorbis;
    AlignOggVorbis(&oggvorbis, vf);
    int32_t ret = my->ov_clear(&oggvorbis);
    UnalignOggVorbis(vf, &oggvorbis);
    return ret;
}
EXPORT int32_t my_ov_comment(x86emu_t* emu, void* vf, int32_t i) {
    OggVorbis oggvorbis;
    AlignOggVorbis(&oggvorbis, vf);
    return my->ov_comment(&oggvorbis, i);
}
EXPORT int32_t my_ov_crosslap(x86emu_t* emu, void* vf, void* v2) {
    OggVorbis oggvorbis, ov2;
    AlignOggVorbis(&oggvorbis, vf);
    AlignOggVorbis(&ov2, v2);
    int32_t ret = my->ov_crosslap(&oggvorbis, &ov2);
    UnalignOggVorbis(vf, &oggvorbis);
    UnalignOggVorbis(v2, &ov2);
    return ret;
}
EXPORT int32_t my_ov_fopen(x86emu_t* emu, void* p, void* vf) {
    OggVorbis oggvorbis;
    AlignOggVorbis(&oggvorbis, vf);
    int32_t ret = my->ov_fopen(p, &oggvorbis);
    UnalignOggVorbis(vf, &oggvorbis);
    return ret;
}
EXPORT int32_t my_ov_halfrate(x86emu_t* emu, void* vf, int32_t i) {
    OggVorbis oggvorbis;
    AlignOggVorbis(&oggvorbis, vf);
    int32_t ret = my->ov_halfrate(&oggvorbis, i);
    UnalignOggVorbis(vf, &oggvorbis);
    return ret;
}
EXPORT int32_t my_ov_halfrate_p(x86emu_t* emu, void* vf) {
    OggVorbis oggvorbis;
    AlignOggVorbis(&oggvorbis, vf);
    int32_t ret = my->ov_halfrate_p(&oggvorbis);
    UnalignOggVorbis(vf, &oggvorbis);
    return ret;
}
EXPORT void* my_ov_info(x86emu_t* emu, void* vf, int32_t i) {
    OggVorbis oggvorbis;
    AlignOggVorbis(&oggvorbis, vf);
    void* ret = my->ov_info(&oggvorbis, i);
    UnalignOggVorbis(vf, &oggvorbis);
    return ret;
}
EXPORT int32_t my_ov_open(x86emu_t* emu, void* f, void* vf, void* init, int32_t i) {
    OggVorbis oggvorbis;
    AlignOggVorbis(&oggvorbis, vf);
    int32_t ret = my->ov_open(f, &oggvorbis, init, i);
    UnalignOggVorbis(vf, &oggvorbis);
    return ret;
}
EXPORT int32_t my_ov_pcm_seek(x86emu_t* emu, void* vf, int64_t i) {
    OggVorbis oggvorbis;
    AlignOggVorbis(&oggvorbis, vf);
    int32_t ret = my->ov_pcm_seek(&oggvorbis, i);
    UnalignOggVorbis(vf, &oggvorbis);
    return ret;
}
EXPORT int32_t my_ov_pcm_seek_lap(x86emu_t* emu, void* vf, int64_t i) {
    OggVorbis oggvorbis;
    AlignOggVorbis(&oggvorbis, vf);
    int32_t ret = my->ov_pcm_seek_lap(&oggvorbis, i);
    UnalignOggVorbis(vf, &oggvorbis);
    return ret;
}
EXPORT int32_t my_ov_pcm_seek_page(x86emu_t* emu, void* vf, int64_t i) {
    OggVorbis oggvorbis;
    AlignOggVorbis(&oggvorbis, vf);
    int32_t ret = my->ov_pcm_seek_page(&oggvorbis, i);
    UnalignOggVorbis(vf, &oggvorbis);
    return ret;
}
EXPORT int32_t my_ov_pcm_seek_page_lap(x86emu_t* emu, void* vf, int64_t i) {
    OggVorbis oggvorbis;
    AlignOggVorbis(&oggvorbis, vf);
    int32_t ret = my->ov_pcm_seek_page_lap(&oggvorbis, i);
    UnalignOggVorbis(vf, &oggvorbis);
    return ret;
}
EXPORT int64_t my_ov_pcm_tell(x86emu_t* emu, void* vf) {
    OggVorbis oggvorbis;
    AlignOggVorbis(&oggvorbis, vf);
    int64_t ret = my->ov_pcm_tell(&oggvorbis);
    UnalignOggVorbis(vf, &oggvorbis);
    return ret;
}
EXPORT int64_t my_ov_pcm_total(x86emu_t* emu, void* vf, int32_t i) {
    OggVorbis oggvorbis;
    AlignOggVorbis(&oggvorbis, vf);
    int64_t ret = my->ov_pcm_total(&oggvorbis, i);
    UnalignOggVorbis(vf, &oggvorbis);
    return ret;
}
EXPORT int32_t my_ov_raw_seek(x86emu_t* emu, void* vf, int32_t i) {
    OggVorbis oggvorbis;
    AlignOggVorbis(&oggvorbis, vf);
    int32_t ret = my->ov_raw_seek(&oggvorbis, i);
    UnalignOggVorbis(vf, &oggvorbis);
    return ret;
}
EXPORT int32_t my_ov_raw_seek_lap(x86emu_t* emu, void* vf, int32_t i) {
    OggVorbis oggvorbis;
    AlignOggVorbis(&oggvorbis, vf);
    int32_t ret = my->ov_raw_seek_lap(&oggvorbis, i);
    UnalignOggVorbis(vf, &oggvorbis);
    return ret;
}
EXPORT int64_t my_ov_raw_tell(x86emu_t* emu, void* vf) {
    OggVorbis oggvorbis;
    AlignOggVorbis(&oggvorbis, vf);
    int64_t ret = my->ov_raw_tell(&oggvorbis);
    UnalignOggVorbis(vf, &oggvorbis);
    return ret;
}
EXPORT int64_t my_ov_raw_total(x86emu_t* emu, void* vf, int32_t i) {
    OggVorbis oggvorbis;
    AlignOggVorbis(&oggvorbis, vf);
    int64_t ret = my->ov_raw_total(&oggvorbis, i);
    UnalignOggVorbis(vf, &oggvorbis);
    return ret;
}
EXPORT int32_t my_ov_read(x86emu_t* emu, void* vf, void* buff, int32_t l, int32_t b, int32_t w, int32_t s, void* bs) {
    OggVorbis oggvorbis;
    AlignOggVorbis(&oggvorbis, vf);
    int32_t ret = my->ov_read(&oggvorbis, buff, l, b, w, s, bs);
    UnalignOggVorbis(vf, &oggvorbis);
    return ret;
}
EXPORT int32_t my_ov_read_float(x86emu_t* emu, void* vf, void* buff, int32_t l, void* bs) {
    OggVorbis oggvorbis;
    AlignOggVorbis(&oggvorbis, vf);
    int32_t ret = my->ov_read_float(&oggvorbis, buff, l, bs);
    UnalignOggVorbis(vf, &oggvorbis);
    return ret;
}
EXPORT int32_t my_ov_seekable(x86emu_t* emu, void* vf) {
    OggVorbis oggvorbis;
    AlignOggVorbis(&oggvorbis, vf);
    int32_t ret = my->ov_seekable(&oggvorbis);
    UnalignOggVorbis(vf, &oggvorbis);
    return ret;
}
EXPORT int32_t my_ov_serialnumber(x86emu_t* emu, void* vf, int32_t i) {
    OggVorbis oggvorbis;
    AlignOggVorbis(&oggvorbis, vf);
    int32_t ret = my->ov_serialnumber(&oggvorbis, i);
    UnalignOggVorbis(vf, &oggvorbis);
    return ret;
}
EXPORT int32_t my_ov_streams(x86emu_t* emu, void* vf) {
    OggVorbis oggvorbis;
    AlignOggVorbis(&oggvorbis, vf);
    int32_t ret = my->ov_streams(&oggvorbis);
    UnalignOggVorbis(vf, &oggvorbis);
    return ret;
}
EXPORT int32_t my_ov_test(x86emu_t* emu, void* p, void* vf, void* bs, int32_t l) {
    OggVorbis oggvorbis;
    AlignOggVorbis(&oggvorbis, vf);
    int32_t ret = my->ov_test(p, &oggvorbis, bs, l);
    UnalignOggVorbis(vf, &oggvorbis);
    return ret;
}
EXPORT int32_t my_ov_test_open(x86emu_t* emu, void* vf) {
    OggVorbis oggvorbis;
    AlignOggVorbis(&oggvorbis, vf);
    int32_t ret = my->ov_test_open(&oggvorbis);
    UnalignOggVorbis(vf, &oggvorbis);
    return ret;
}
EXPORT int32_t my_ov_time_seek(x86emu_t* emu, void* vf, double d) {
    OggVorbis oggvorbis;
    AlignOggVorbis(&oggvorbis, vf);
    int32_t ret = my->ov_time_seek(&oggvorbis, d);
    UnalignOggVorbis(vf, &oggvorbis);
    return ret;
}
EXPORT int32_t my_ov_time_seek_lap(x86emu_t* emu, void* vf, double d) {
    OggVorbis oggvorbis;
    AlignOggVorbis(&oggvorbis, vf);
    int32_t ret = my->ov_time_seek_lap(&oggvorbis, d);
    UnalignOggVorbis(vf, &oggvorbis);
    return ret;
}
EXPORT int32_t my_ov_time_seek_page(x86emu_t* emu, void* vf, double d) {
    OggVorbis oggvorbis;
    AlignOggVorbis(&oggvorbis, vf);
    int32_t ret = my->ov_time_seek_page(&oggvorbis, d);
    UnalignOggVorbis(vf, &oggvorbis);
    return ret;
}
EXPORT int32_t my_ov_time_seek_page_lap(x86emu_t* emu, void* vf, double d) {
    OggVorbis oggvorbis;
    AlignOggVorbis(&oggvorbis, vf);
    int32_t ret = my->ov_time_seek_page_lap(&oggvorbis, d);
    UnalignOggVorbis(vf, &oggvorbis);
    return ret;
}
EXPORT double my_ov_time_tell(x86emu_t* emu, void* vf) {
    OggVorbis oggvorbis;
    AlignOggVorbis(&oggvorbis, vf);
    double ret = my->ov_time_tell(&oggvorbis);
    UnalignOggVorbis(vf, &oggvorbis);
    return ret;
}
EXPORT double my_ov_time_total(x86emu_t* emu, void* vf, int32_t i) {
    OggVorbis oggvorbis;
    AlignOggVorbis(&oggvorbis, vf);
    double ret = my->ov_time_total(&oggvorbis, i);
    UnalignOggVorbis(vf, &oggvorbis);
    return ret;
}
#endif  //!NOALIGN

#define SUPER() \
GO(0)           \
GO(1)           \
GO(2)           \
GO(3)           \
GO(4)           \
GO(5)           \
GO(6)           \
GO(7)

// read
#define GO(A)   \
static uintptr_t my_read_fct_##A = 0;                                                                       \
static unsigned long my_read_##A(void* ptr, unsigned long size, unsigned long nmemb, void* datasource)      \
{                                                                                                           \
    return (unsigned long)RunFunctionFmt(my_read_fct_##A, "pLLp", ptr, size, nmemb, datasource);\
}
SUPER()
#undef GO
static void* findreadFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_read_fct_##A == (uintptr_t)fct) return my_read_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_read_fct_##A == 0) {my_read_fct_##A = (uintptr_t)fct; return my_read_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for vorbisfile read callback\n");
    return NULL;
}
// seek
#define GO(A)   \
static uintptr_t my_seek_fct_##A = 0;                                                   \
static int my_seek_##A(void* ptr, int64_t offset, int whence)                           \
{                                                                                       \
    return (int)RunFunctionFmt(my_seek_fct_##A, "pIi", ptr, offset, whence);\
}
SUPER()
#undef GO
static void* findseekFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_seek_fct_##A == (uintptr_t)fct) return my_seek_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_seek_fct_##A == 0) {my_seek_fct_##A = (uintptr_t)fct; return my_seek_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for vorbisfile seek callback\n");
    return NULL;
}
// close
#define GO(A)   \
static uintptr_t my_close_fct_##A = 0;                                  \
static int my_close_##A(void* ptr)                                      \
{                                                                       \
    return (int)RunFunctionFmt(my_close_fct_##A, "p", ptr); \
}
SUPER()
#undef GO
static void* findcloseFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_close_fct_##A == (uintptr_t)fct) return my_close_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_close_fct_##A == 0) {my_close_fct_##A = (uintptr_t)fct; return my_close_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for vorbisfile close callback\n");
    return NULL;
}
// tell
#define GO(A)   \
static uintptr_t my_tell_fct_##A = 0;                                   \
static long my_tell_##A(void* ptr)                                      \
{                                                                       \
    return (long)RunFunctionFmt(my_tell_fct_##A, "p", ptr); \
}
SUPER()
#undef GO
static void* findtellFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_tell_fct_##A == (uintptr_t)fct) return my_tell_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_tell_fct_##A == 0) {my_tell_fct_##A = (uintptr_t)fct; return my_tell_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for vorbisfile tell callback\n");
    return NULL;
}

#undef SUPER

EXPORT int32_t my_ov_open_callbacks(x86emu_t* emu, void* datasource, void* vf, void* initial, long ibytes, void* read_fnc, void* seek_fnc, void* close_fnc, void* tell_fnc)
{
    (void)emu;
    ov_callbacks cbs = {0};
    cbs.read_func = findreadFct(read_fnc);
    cbs.seek_func = findseekFct(seek_fnc);
    cbs.close_func = findcloseFct(close_fnc);
    cbs.tell_func = findtellFct(tell_fnc);
    OggVorbis oggvorbis;
    AlignOggVorbis(&oggvorbis, vf);
    int32_t ret =  my->ov_open_callbacks(datasource, &oggvorbis, initial, ibytes, cbs);
    UnalignOggVorbis(vf, &oggvorbis);
    return ret;
}

#ifdef PANDORA
// No really ok, because it will depends on the order of initialisation
#define PRE_INIT \
    vorbisfileName = (box86->sdl1mixerlib || box86->sdl2mixerlib)?vorbisfileNameAlt:vorbisfileNameReg;
#endif

#define CUSTOM_INIT \
    getMy(lib);

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"

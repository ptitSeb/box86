/*******************************************************************
 * File automatically generated by rebuild_wrappers.py (v2.0.0.11) *
 *******************************************************************/
#ifndef __wrappedlibvorbisTYPES_H_
#define __wrappedlibvorbisTYPES_H_

#ifndef LIBNAME
#error You should only #include this file inside a wrapped*.c file
#endif
#ifndef ADDED_FUNCTIONS
#define ADDED_FUNCTIONS() 
#endif

typedef void (*vFp_t)(void*);
typedef int32_t (*iFp_t)(void*);
typedef int32_t (*iFpi_t)(void*, int32_t);
typedef int32_t (*iFpp_t)(void*, void*);
typedef void* (*pFpi_t)(void*, int32_t);
typedef int32_t (*iFppppp_t)(void*, void*, void*, void*, void*);

#define SUPER() ADDED_FUNCTIONS() \
	GO(vorbis_dsp_clear, vFp_t) \
	GO(vorbis_bitrate_addblock, iFp_t) \
	GO(vorbis_block_clear, iFp_t) \
	GO(vorbis_synthesis_restart, iFp_t) \
	GO(vorbis_analysis_wrote, iFpi_t) \
	GO(vorbis_synthesis_read, iFpi_t) \
	GO(vorbis_analysis, iFpp_t) \
	GO(vorbis_analysis_blockout, iFpp_t) \
	GO(vorbis_analysis_init, iFpp_t) \
	GO(vorbis_bitrate_flushpacket, iFpp_t) \
	GO(vorbis_block_init, iFpp_t) \
	GO(vorbis_synthesis, iFpp_t) \
	GO(vorbis_synthesis_blockin, iFpp_t) \
	GO(vorbis_synthesis_init, iFpp_t) \
	GO(vorbis_synthesis_lapout, iFpp_t) \
	GO(vorbis_synthesis_pcmout, iFpp_t) \
	GO(vorbis_synthesis_trackonly, iFpp_t) \
	GO(vorbis_analysis_buffer, pFpi_t) \
	GO(vorbis_window, pFpi_t) \
	GO(vorbis_analysis_headerout, iFppppp_t)

#endif // __wrappedlibvorbisTYPES_H_

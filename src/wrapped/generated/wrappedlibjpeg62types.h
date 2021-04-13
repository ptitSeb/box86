/*****************************************************************
 * File automatically generated by rebuild_wrappers.py (v1.2.0.09)
 *****************************************************************/
#ifndef __wrappedlibjpeg62TYPES_H_
#define __wrappedlibjpeg62TYPES_H_

#ifndef LIBNAME
#error You should only #include this file inside a wrapped*.c file
#endif
#ifndef ADDED_FUNCTIONS
#define ADDED_FUNCTIONS() 
#endif

typedef void (*vFp_t)(void*);
typedef int32_t (*iFp_t)(void*);
typedef void* (*pFp_t)(void*);
typedef void (*vFpi_t)(void*, int32_t);
typedef int32_t (*iFpi_t)(void*, int32_t);
typedef void (*vFpii_t)(void*, int32_t, int32_t);
typedef void (*vFpiL_t)(void*, int32_t, uintptr_t);
typedef void (*vFpip_t)(void*, int32_t, void*);
typedef void (*vFppp_t)(void*, void*, void*);
typedef uint32_t (*uFppu_t)(void*, void*, uint32_t);
typedef void (*vFpipu_t)(void*, int32_t, void*, uint32_t);

#define SUPER() ADDED_FUNCTIONS() \
	GO(jpeg_destroy_compress, vFp_t) \
	GO(jpeg_destroy_decompress, vFp_t) \
	GO(jpeg_finish_compress, vFp_t) \
	GO(jpeg_set_defaults, vFp_t) \
	GO(jpeg_finish_decompress, iFp_t) \
	GO(jpeg_start_decompress, iFp_t) \
	GO(jpeg_std_error, pFp_t) \
	GO(jpeg_start_compress, vFpi_t) \
	GO(jpeg_read_header, iFpi_t) \
	GO(jpeg_resync_to_restart, iFpi_t) \
	GO(jpeg_set_quality, vFpii_t) \
	GO(jpeg_CreateCompress, vFpiL_t) \
	GO(jpeg_CreateDecompress, vFpiL_t) \
	GO(jpeg_set_marker_processor, vFpip_t) \
	GO(jpeg_mem_dest, vFppp_t) \
	GO(jpeg_read_scanlines, uFppu_t) \
	GO(jpeg_write_scanlines, uFppu_t) \
	GO(jpeg_write_marker, vFpipu_t)

#endif // __wrappedlibjpeg62TYPES_H_

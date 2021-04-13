/*****************************************************************
 * File automatically generated by rebuild_wrappers.py (v1.2.0.09)
 *****************************************************************/
#ifndef __wrappedlibxcbrandrTYPES_H_
#define __wrappedlibxcbrandrTYPES_H_

#ifndef LIBNAME
#error You should only #include this file inside a wrapped*.c file
#endif
#ifndef ADDED_FUNCTIONS
#define ADDED_FUNCTIONS() 
#endif

typedef my_xcb_iterator_t (*TFp_t)(void*);
typedef my_xcb_cookie_t (*XFpu_t)(void*, uint32_t);
typedef my_xcb_cookie_t (*XFpuW_t)(void*, uint32_t, uint16_t);
typedef my_xcb_cookie_t (*XFpuu_t)(void*, uint32_t, uint32_t);
typedef my_xcb_cookie_t (*XFppu_t)(void*, void*, uint32_t);

#define SUPER() ADDED_FUNCTIONS() \
	GO(xcb_randr_get_screen_resources_current_outputs_end, TFp_t) \
	GO(xcb_randr_get_output_primary, XFpu_t) \
	GO(xcb_randr_get_output_primary_unchecked, XFpu_t) \
	GO(xcb_randr_get_screen_resources, XFpu_t) \
	GO(xcb_randr_get_screen_resources_current, XFpu_t) \
	GO(xcb_randr_get_screen_resources_outputs, XFpu_t) \
	GO(xcb_randr_get_screen_resources_unchecked, XFpu_t) \
	GO(xcb_randr_select_input, XFpuW_t) \
	GO(xcb_randr_select_input_checked, XFpuW_t) \
	GO(xcb_randr_query_version, XFpuu_t) \
	GO(xcb_randr_query_version_unchecked, XFpuu_t) \
	GO(xcb_randr_get_crtc_info, XFppu_t) \
	GO(xcb_randr_get_crtc_info_unchecked, XFppu_t) \
	GO(xcb_randr_get_output_info, XFppu_t) \
	GO(xcb_randr_get_output_info_unchecked, XFppu_t)

#endif // __wrappedlibxcbrandrTYPES_H_

#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

//%S x my_xcb_cookie_t u

GO(xcb_create_pixmap_from_bitmap_data, pFpupuuuuup)
GO(xcb_image_annotate, vFp)
GO(xcb_image_convert, pFpp)
GO(xcb_image_create, pFWWiCCCCiipup)
GO(xcb_image_create_from_bitmap_data, pFpuu)
GO(xcb_image_create_native, pFpWWiCpup)
GO(xcb_image_destroy, vFp)
GO(xcb_image_get, pFpdwwWWui)
GO(xcb_image_get_pixel, uFpuu)
GO(xcb_image_native, pFppi)
GOS(xcb_image_put, xFEpuupwwC)
GO(xcb_image_put_pixel, vFpuuu)
GO(xcb_image_shm_get, iFpupuupWWu)    // xcb_shm_segment_info_t is a struct with "u u p" (TODO: add a structure?)
GO(xcb_image_shm_put, pFpuupuupwwwwWWC)
GO(xcb_image_subimage, pFpuuuupup)

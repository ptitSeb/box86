#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

//%S x my_xcb_cookie_t u
//%S X my_xcb_iterator_t pii

GOS(xcb_shm_attach, xFEpuuC)
GOS(xcb_shm_attach_checked, xFEpuuC)
GOS(xcb_shm_attach_fd, xFEpuuC)
GOS(xcb_shm_attach_fd_checked, xFEpuuC)
GOS(xcb_shm_create_pixmap, xFEpuuWWCuu)
GOS(xcb_shm_create_pixmap_checked, xFEpuuWWCuu)
GOS(xcb_shm_create_segment, xFEpuuC)
GO(xcb_shm_create_segment_reply, pFpup)
GO(xcb_shm_create_segment_reply_fds, pFpp)
GOS(xcb_shm_create_segment_unchecked, xFEpuuC)
GOS(xcb_shm_detach, xFEpu)
GOS(xcb_shm_detach_checked, xFEpu)
GOS(xcb_shm_get_image, xFEpuwwWWuCuu)
GO(xcb_shm_get_image_reply, pFpup)
GOS(xcb_shm_get_image_unchecked, xFEpuwwWWuCuu)
DATA(xcb_shm_id, 8)
GOS(xcb_shm_put_image, xFEpuuWWWWWWwwCCCuu)
GOS(xcb_shm_put_image_checked, xFEpuuWWWWWWwwCCCuu)
GOS(xcb_shm_query_version, xFEp)
GO(xcb_shm_query_version_reply, pFpup)
GOS(xcb_shm_query_version_unchecked, xFEp)
GOS(xcb_shm_seg_end, XFEX)
GO(xcb_shm_seg_next, vFp)

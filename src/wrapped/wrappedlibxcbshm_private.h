#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

//%S X my_xcb_cookie_t
//%S T my_xcb_iterator_t

GOS(xcb_shm_attach, pFEppuuC) //%X
GOS(xcb_shm_attach_checked, pFEppuuC) //%X
GOS(xcb_shm_attach_fd, pFEppuuC) //%X
GOS(xcb_shm_attach_fd_checked, pFEppuuC) //%X
GOS(xcb_shm_create_pixmap, pFEppuuWWCuu) //%X
GOS(xcb_shm_create_pixmap_checked, pFEppuuWWCuu) //%X
GOS(xcb_shm_create_segment, pFEppuuC) //%X
GO(xcb_shm_create_segment_reply, pFpup)
GO(xcb_shm_create_segment_reply_fds, pFpp)
GOS(xcb_shm_create_segment_unchecked, pFEppuuC) //%X
GOS(xcb_shm_detach, pFEppu) //%X
GOS(xcb_shm_detach_checked, pFEppu) //%X
GOS(xcb_shm_get_image, pFEppuwwWWuCuu) //%X
GO(xcb_shm_get_image_reply, pFpup)
GOS(xcb_shm_get_image_unchecked, pFEppuwwWWuCuu) //%X
DATA(xcb_shm_id, 8)
GOS(xcb_shm_put_image, pFEppuuWWWWWWwwCCCuu) //%X
GOS(xcb_shm_put_image_checked, pFEppuuWWWWWWwwCCCuu) //%X
GOS(xcb_shm_query_version, pFEpp) //%X
GO(xcb_shm_query_version_reply, pFpup)
GOS(xcb_shm_query_version_unchecked, pFEpp) //%X
GOS(xcb_shm_seg_end, pFEppii)   //%{TFT} xcb_generic_iterator_t by value, so "pii"
GO(xcb_shm_seg_next, vFp)

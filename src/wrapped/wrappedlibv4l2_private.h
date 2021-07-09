#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

GO(v4l2_open, iFpi)
GO(v4l2_close, iFi)
GO(v4l2_ioctl, iFiii)
GO(v4l2_read, iFipu)
GOM(v4l2_mmap, pFEpLiiii)
GOM(v4l2_munmap, iFEpL)

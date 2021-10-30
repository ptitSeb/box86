#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

GO(scalable_aligned_free, vFp)
GO(scalable_aligned_malloc, pFLL)
GO(scalable_aligned_realloc, pFpLL)
GO(scalable_allocation_command, iFip)
GO(scalable_allocation_mode, iFil)
GO(scalable_calloc, pFLL)
GO(scalable_free, vFp)
GO(scalable_malloc, pFL)
GO(scalable_msize, LFp)
GO(scalable_posix_memalign, iFpLL)
GO(scalable_realloc, pFpL)

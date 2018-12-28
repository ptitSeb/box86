#ifdef GO

GO(uFv)
GO(uFE, emu)
GO(uFp, p(0))
GO(uFpuup, p(0), u32(4), u32(8), p(12))
GO(uFpupp, p(0), u32(4), p(8), p(12))
GO(uFpuppu, p(0), u32(4), p(8), p(12), u32(16))

#else
#error Meh
#endif
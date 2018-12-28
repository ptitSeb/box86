#ifdef GO

GO(uFv)
GO(uFE, emu)
GO(uFp, p(0))
GO(uFpp, p(0), p(4))
GO(uFpupp, p(0), u32(4), p(8), p(12))
GO(uFpuup, p(0), u32(4), u32(8), p(12))
GO(uFppiip, p(0), p(4), i32(8), i32(12), p(16))
GO(uFpuppu, p(0), u32(4), p(8), p(12), u32(16))

#else
#error Meh
#endif
#ifdef GO

GO(pFv)
GO(pFp, p(0))
GO(pFu, u32(0))
GO(pFE, emu)
GO(pFEp, emu, p(0))
GO(pFuu, u32(0), u32(4))
GO(pFip, i32(0), p(4))
GO(pFpi, p(0), i32(4))
GO(pFpp, p(0), p(4))
GO(pFEpi, emu, p(0), i32(4))
GO(pFEpp, emu, p(0), p(4))
GO(pFppi, p(0), p(4), i32(8))
GO(pFppu, p(0), p(4), u32(8))
GO(pFppp, p(0), p(4), p(8))
GO(pFEppp, emu, p(0), p(4), p(8))
GO(pFpippp, p(0), i32(4), p(8), p(12), p(16))
GO(pFpiiiiu, p(0), i32(4), i32(8), i32(12), i32(16), u32(20))
GO(pFppiiuuui, p(0), p(4), i32(8), i32(12), u32(16), u32(20), u32(24), i32(28))
GO(pFppiiuuuipii, p(0), p(4), i32(8), i32(12), u32(16), u32(20), u32(24), i32(28), p(32), i32(36), i32(40))

#else
#error Meh
#endif
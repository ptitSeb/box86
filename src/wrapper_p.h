#ifdef GO

GO(pFv)
GO(pFp, p(0))
GO(pFu, u32(0))
GO(pFuu, u32(0), u32(4))
GO(pFpp, p(0), p(4))
GO(pFppu, p(0), p(4), u32(8))
GO(pFppp, p(0), p(4), p(8))

#else
#error Meh
#endif